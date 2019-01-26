#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <string>
#include <map>

#include "Result.h"

struct JoystickOutput {
    float value = 0;
    double time = 0;
    double time_total = 0;
    bool pressed = false;
    bool pressed_new = false;
    double last_time_total = 0;
};

class Joystick {
    public:
        Error load(const std::string& path);

        void update(double t);

        bool isCompatible(int glfw_id);
        void connect(int glfw_id);

        float getValue(const std::string& name);
        float getTime(const std::string& name);
        float getTimeTotal(const std::string& name);
        bool getPressed(const std::string& name);
        bool getPressedNew(const std::string& name);

        const std::map<std::string, JoystickOutput>& getOutputs() const;

    private:
        void setJoystickOutput(const std::string& name, bool pressed, double t, float v);

        bool isAxisPressed(const float* axes, int i, int sibling=-1);
        int getStickSibling(int i);

        std::map<std::string, double> press_start_;
        int glfw_id_ = -1;

        std::string device_;
        const std::string path_;
        std::map<int, float> axis_neutrals_;
        std::map<int, std::string> axis_names_;
        std::map<int, std::string> button_names_;
        std::map<int, int> stick_siblings_;
        std::map<std::string, std::string> fake_buttons_negative_;
        std::map<std::string, std::string> fake_buttons_positive_;
        float deadzone_ = 0;

        float getAxisNeutral(int i);
        std::string getButtonName(int i);
        std::string getAxisName(int i);

        std::map<std::string, JoystickOutput> outputs_;

        std::map<std::string, bool> triggers_;
};

#endif
