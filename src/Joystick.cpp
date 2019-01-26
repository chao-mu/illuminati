#include "Joystick.h"

#include "yaml-cpp/yaml.h"
#include "GLFW/glfw3.h"

#include <stdexcept>

#include "MathUtil.h"

#define AXIS_LOW -1
#define AXIS_HIGH 1
#define TRIGGER_LOW 0
#define TRIGGER_HIGH 1

void Joystick::connect(int glfw_id) {
    glfw_id_ = glfw_id;
}

const std::map<std::string, JoystickOutput>& Joystick::getOutputs() const {
    return outputs_;
}

Error Joystick::load(const std::string& path) {
    YAML::Node config;

    try {
        config = YAML::LoadFile(path);
    } catch (std::runtime_error& err) {
        return err.what();
    }

    device_ = config["device"].as<std::string>();
    deadzone_ = config["deadzone"].as<float>();

    for (YAML::const_iterator it=config["neutral"].begin(); it != config["neutral"].end(); ++it) {
        axis_neutrals_[it->first.as<int>()] = it->second.as<float>();
    }

    for (YAML::const_iterator it=config["stick_siblings"].begin(); it != config["stick_siblings"].end(); ++it) {
        stick_siblings_[it->first.as<int>()] = it->second.as<int>();
    }

    for (YAML::const_iterator it=config["axes"].begin(); it != config["axes"].end(); ++it) {
        std::string name = it->second.as<std::string>();
        axis_names_[it->first.as<int>()] = name;

        outputs_[name] = JoystickOutput{};
    }

    for (YAML::const_iterator it=config["buttons"].begin(); it != config["buttons"].end(); ++it) {
        std::string name = it->second.as<std::string>();
        button_names_[it->first.as<int>()] = name;
        outputs_[name] = JoystickOutput{};
    }

    for (YAML::const_iterator it=config["fake_buttons_negative"].begin(); it != config["fake_buttons_negative"].end(); ++it) {
        std::string name = it->first.as<std::string>();
        fake_buttons_negative_[name] = it->second.as<std::string>();
        outputs_[name] = JoystickOutput{};
    }

    for (YAML::const_iterator it=config["fake_buttons_positive"].begin(); it != config["fake_buttons_positive"].end(); ++it) {
        std::string name = it->first.as<std::string>();
        fake_buttons_positive_[name] = it->second.as<std::string>();
        outputs_[name] = JoystickOutput{};
    }

    for (const auto& node : config["triggers"]) {
        triggers_[node.as<std::string>()] = true;
    }

    return {};
}

void Joystick::setJoystickOutput(const std::string& name, bool pressed, double t, float v) {
    //printf("%s: %i %f %f\n", name.c_str(), pressed, t, v);
    JoystickOutput& output = outputs_.at(name);
    if (pressed) {
        output.pressed = true;
        output.value = v;

        if (press_start_.count(name)) {
            output.pressed_new = false;
        } else {
            output.pressed_new = true;
            press_start_[name] = t;
        }

        output.time = t - press_start_[name];
        output.time_total = output.time + output.last_time_total;
    } else {
        press_start_.erase(name);

        if (output.pressed) {
            output.last_time_total = output.time_total;
        }

        output.pressed_new = false;
        output.pressed = false;
        output.value = 0;
        output.time = 0;
    }
}

void Joystick::update(double t) {
    if (glfwJoystickPresent(glfw_id_) != GLFW_TRUE) {
        return;
    }

    int axes_count;
    const float* axes = glfwGetJoystickAxes(glfw_id_, &axes_count);
    for (int i=0; i < axes_count; i++) {
        const std::string name = getAxisName(i);
        if (name == "") {
            continue;
        }

        float v = axes[i];
        float deadzone = deadzone_;
        float neutral = getAxisNeutral(i);
        float adjusted_low = -1 + deadzone;
        float adjusted_high = 1 - deadzone;
        bool adjusted = false;
        bool pressed = isAxisPressed(axes, i, getStickSibling(i));
        if (pressed) {
            if (v - neutral < -deadzone) {
                v += deadzone;
                if (v > adjusted_high) {
                    v = adjusted_high;
                }

                adjusted = true;
            } else if (v - neutral > deadzone) {
                v -= deadzone;
                if (v < adjusted_low) {
                    v = adjusted_low;
                }

                adjusted = true;
            } else {
                v = neutral;
            }

            // Remap to desired range
            if (adjusted) {
                if (triggers_.count(name)) {
                    v = remap(v, adjusted_low, adjusted_high, TRIGGER_LOW, TRIGGER_HIGH);
                } else {
                    v = remap(v, adjusted_low, adjusted_high, AXIS_LOW, AXIS_HIGH);
                }
            }
        } else {
            v = 0;
        }

        setJoystickOutput(name, pressed, t, v);
    }

    int button_count;
    const unsigned char* buttons = glfwGetJoystickButtons(glfw_id_, &button_count);
    for (int i=0; i < button_count; i++) {
        const std::string name = getButtonName(i);
        if (name == "") {
            continue;
        }

        bool pressed = buttons[i] == GLFW_PRESS;
        setJoystickOutput(name, pressed, t, pressed ? 1 : 0);
    }

    for (const auto& kv : fake_buttons_negative_) {
        std::string alias = kv.first;
        std::string axis_name = kv.second;

        if (!outputs_.count(axis_name)) {
            throw std::out_of_range("Was expecting module for joystick " + device_ + " to define output " + axis_name);
        }

        bool pressed = outputs_.at(axis_name).value < 0;
        setJoystickOutput(alias, pressed, t, pressed ? 1 : 0);
    }

    for (const auto& kv : fake_buttons_positive_) {
        std::string alias = kv.first;
        std::string axis_name = kv.second;

        if (!outputs_.count(axis_name)) {
            throw std::out_of_range("Was expecting module for joystick " + device_ + " to define output " + axis_name);
        }

        bool pressed = outputs_.at(axis_name).value > 0;
        setJoystickOutput(alias, pressed, t, pressed ? 1 : 0);
    }
}

bool Joystick::isCompatible(int glfw_id) {
    return std::string(glfwGetJoystickName(glfw_id)).find(device_) != std::string::npos;
}

bool Joystick::isAxisPressed(const float* axes, int i, int sibling) {
    float v = axes[i];
    float deadzone = deadzone_;
    float neutral = getAxisNeutral(i);

    bool pressed = (v - neutral < -deadzone) || (v - neutral > deadzone);
    if (sibling >= 0) {
        pressed |= isAxisPressed(axes, sibling);
    }

    return pressed;
}

int Joystick::getStickSibling(int i) {
    if (stick_siblings_.count(i) > 0) {
        return stick_siblings_.at(i);
    }

    return -1;
}

float Joystick::getAxisNeutral(int i) {
    if (axis_neutrals_.count(i) > 0) {
        return axis_neutrals_.at(i);
    }

    return 0;
}

std::string Joystick::getButtonName(int i) {
    if (!button_names_.count(i)) {
        return "";
    }

    return button_names_.at(i);
}

std::string Joystick::getAxisName(int i) {
    if (!axis_names_.count(i)) {
        return "";
    }

    return axis_names_.at(i);
}

#undef LOG_JOYSTICK
#undef AXIS_LOW
#undef AXIS_HIGH
#undef TRIGGER_LOW
#undef TRIGGER_HIGH
