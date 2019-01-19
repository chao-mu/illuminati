#include <memory>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "App.h"

int main(int /* argc */, char** /* argv */) {
    std::unique_ptr<App> app = std::make_unique<App>();

    glfwSetErrorCallback(app->onError);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW!!!\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Awesome Demo", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "Failed to create window\n");
        return -1;
    }

    glfwSetWindowSizeCallback(window, App::onWindowSize);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    std::optional<std::string> err = app->setup();
    if (err.has_value()) {
        std::cerr << "Error initializing app: " << err.value() << std::endl;
        return -1;
    }

    float last_frame = -1;
    float per_frame = 1 / 30.;
    while (!glfwWindowShouldClose(window)) {
        float t = glfwGetTime();
        glfwPollEvents();
        if (last_frame < 0 || t - last_frame > per_frame) {
            app->draw(window, t);
            last_frame = t;
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


/*
 * checkout http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    */
