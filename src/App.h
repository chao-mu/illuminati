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
#include "Image.h"

class App {
    public:
        App(const std::filesystem::path& out_dir, std::pair<int, int> size);
        Error setup(std::filesystem::path vert_path, std::filesystem::path frag_path, std::vector<std::shared_ptr<Joystick>> joysticks, std::filesystem::path& path);
        void draw(GLFWwindow* window, double t);
        void onError(int error, const char* desc);
        void onWindowSize(GLFWwindow* window, int width, int height);
        void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
        Error screenshot();
        bool setupWebcam(int dev);

    private:
        GLuint ebo = GL_FALSE;
        GLuint vao = GL_FALSE;
        GLuint pos_vbo_ = GL_FALSE;
        GLuint coord_vbo_ = GL_FALSE;
        GLuint webcam_tex_ = GL_FALSE;
        GLuint fbo_ = GL_FALSE;

        GLuint output_texs_[2] = {};
        GLenum draw_bufs_[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

        std::unique_ptr<Image> img_;
        std::unique_ptr<ShaderProgram> program_;
        std::unique_ptr<JoystickManager> joy_manager_;
        std::vector<std::shared_ptr<Joystick>> joysticks_;
        std::string last_err_ = "";
        std::string last_warning_ = "";
        const std::filesystem::path out_dir_;
        std::unique_ptr<Webcam> webcam_;
        std::pair<int, int> size_;

};

#endif
