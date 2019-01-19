#ifndef APP_H
#define APP_H

#include <filesystem>
#include <optional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Result.h"
#include "ShaderProgram.h"

class App {
    public:
        Error setup();
        void draw(GLFWwindow* window, float t);
        static void onError(int error, const char* desc);
        static void onWindowSize(GLFWwindow* window, int width, int height);

    private:
        GLuint ebo = 0;
        GLuint vao = 0;
        GLuint vbo = 0;

        std::unique_ptr<ShaderProgram> program_;
        std::string last_err_ = "";
};

#endif
