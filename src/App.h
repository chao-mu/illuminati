#ifndef APP_H
#define APP_H

#include <filesystem>
#include <optional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include "Result.h"
#include "ShaderProgram.h"
#include "Joystick.h"
#include "JoystickManager.h"
#include "Webcam.h"

class App {
    public:
        App(const std::filesystem::path& out_dir);
        Error setup(std::filesystem::path vert_path, std::filesystem::path frag_path, std::vector<std::shared_ptr<Joystick>> joysticks);
        void draw(GLFWwindow* window, double t);
        void onError(int error, const char* desc);
        void onWindowSize(GLFWwindow* window, int width, int height);
        void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
        Error writeFBO(GLFWwindow* window, const std::filesystem::path& path);
        Error screenshot(GLFWwindow* window);
        bool setupWebcam(int dev);

    private:
        GLuint ebo = 0;
        GLuint vao = 0;
        GLuint pos_vbo_ = 0;
        GLuint coord_vbo_ = 0;
        GLuint webcam_tex_ = 0;

        std::unique_ptr<ShaderProgram> program_;
        std::unique_ptr<JoystickManager> joy_manager_;
        std::vector<std::shared_ptr<Joystick>> joysticks_;
        std::string last_err_ = "";
        std::string last_warning_ = "";
        const std::filesystem::path out_dir_;
        std::unique_ptr<Webcam> webcam_;
};

#endif
