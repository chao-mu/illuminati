#include "App.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include <cstring>
#include <chrono>

#include "lodepng.h"

#include "Result.h"

App::App(const std::filesystem::path& out_dir) : out_dir_(out_dir) {}

// Write the current frame buffer as a png file at specified path.
Error App::writeFBO(GLFWwindow* window, const std::filesystem::path& path) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    std::vector<unsigned char> image(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

    unsigned errc = lodepng::encode(path, image, width, height);
    if (errc) {
        std::stringstream ss;
        ss << "encoder error " << errc << ": "<< lodepng_error_text(errc);
        return ss.str();
    }

    return {};
}

Error App::screenshot(GLFWwindow* window) {
    std::stringstream s;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::time_t now = std::time(nullptr);
    s << "output-" << std::put_time(std::localtime(&now), "%Y-%m-%d_") << ms << ".png";
    std::filesystem::path dest = out_dir_ / s.str();

    Error err = App::writeFBO(window, dest);
    if (err) {
        return err;
    }

    return {};
}

std::optional<std::string> App::setup(
        std::filesystem::path vert_path,
        std::filesystem::path frag_path,
        std::vector<std::shared_ptr<Joystick>> joysticks
        ) {
    joy_manager_ = std::make_unique<JoystickManager>();
    joysticks_ = joysticks;
    for (auto& joy : joysticks) {
        joy_manager_->addJoystick(joy);
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    GLfloat vertices[] = {
        1.0f,  1.0f, 0.0f,  // top right
        1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  // bottom left
        -1.0f,  1.0f, 0.0f   // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    // Bind vertex array object
    glBindVertexArray(vao);
    // Copy vertices array into vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Copy indices into element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // Set the vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    // Unbind our friends
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Setup shaders
    program_ = std::make_unique<ShaderProgram>();

    Error err = program_->loadShader(GL_VERTEX_SHADER, vert_path);
    if (err.has_value()) {
        return err;
    }

    err = program_->loadShader(GL_FRAGMENT_SHADER, frag_path);
    if (err.has_value()) {
        return err;
    }

    return {};
}

void App::draw(GLFWwindow* window, double t) {
    joy_manager_->update();
    for (auto& joy : joysticks_) {
        joy->update(t);
    }

    std::string err = program_->update().value_or("");
    if (err != "") {
        if (err != last_err_) {
            std::cout << err << std::endl;
            last_err_ = err;
        }
    } else if (last_err_ != "") {
        std::cout << "- No More Errors! - " << std::endl;
        last_err_ = "";
    }

    GLuint program = program_->getProgram();
    // Use our shader
    glUseProgram(program);

    // Set uniforms
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    program_->setUniform("iResolution", [width, height, program](GLint& id) {
        glProgramUniform2f(program, id, (float)width, (float)height);
    });

    program_->setUniform("iTime", [t, program](GLint& id) {
        glProgramUniform1f(program, id, (float)t);
    });

    int i = 1;
    for (const auto& joy : joysticks_) {
        auto outs = joy->getOutputs();
        for (const auto& kv : outs) {
            const std::string& name = kv.first;
            const JoystickOutput& ctrl = kv.second;
            const std::string base = "j" + std::to_string(i) + name;

            program_->setUniform(base + "Pressed", [ctrl, program](GLint& id) {
                glProgramUniform1i(program, id, ctrl.pressed ? 1 : 0);
            });

            program_->setUniform(base + "PressedNew", [ctrl, program](GLint& id) {
                glProgramUniform1i(program, id, ctrl.pressed_new ? 1 : 0);
            });

            program_->setUniform(base + "Time", [ctrl, program](GLint& id) {
                glProgramUniform1f(program, id, (float)ctrl.time);
            });

            program_->setUniform(base + "TimeTotal", [ctrl, program](GLint& id) {
                glProgramUniform1f(program, id, (float)ctrl.time_total);
            });

            program_->setUniform(base, [ctrl, program](GLint& id) {
                glProgramUniform1f(program, id, ctrl.value);
            });
        }

        i++;
    }

    // Draw our vertices
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);

    std::string warning;
    for (const auto& unset : program_->getUnsetUniforms()) {
        warning += "WARNING: unset in-use uniform '" + unset + "'\n";
    }

    if (warning != "") {
        if (warning != last_warning_) {
            // There's a newline at the end of warning
            std::cout << warning << std::flush;
            last_warning_ = warning;
        }
    } else if (last_warning_ != "") {
        std::cout << "- No More Warnings! - " << std::endl;
        last_warning_ = "";
    }
}

void App::onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_P:
                auto err = screenshot(window);
                if (err) std::cerr << "Error screenshotting: " << err.value();
                break;
        }
    }
}

void App::onWindowSize(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0,0, width, height);
}

void App::onError(int /* error */, const char* desc) {
    fputs(desc, stderr);
}
