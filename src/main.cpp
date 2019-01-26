#include <memory>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <tclap/CmdLine.h>
#include <filesystem>

#include "App.h"
#include "Joystick.h"

// #define BENCHMARK

std::unique_ptr<App> app;

static void onError(int error, const char* desc) {
    app->onError(error, desc);
}

static void onWindowSize(GLFWwindow* window, int width, int height) {
    app->onWindowSize(window, width, height);
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    app->onKey(window, key, scancode, action, mods);
}

int main(int argc, char** argv) {
    TCLAP::CmdLine cmd("Illuminati - Everything is Light");

    TCLAP::ValueArg<std::string> vert_arg("", "vert", "path to vertex shader", false, "vert.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> frag_arg("", "frag", "path to fragment shader", false, "frag.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> out_arg("", "out-dir", "path to output directory", false, ".", "string", cmd);
    TCLAP::MultiArg<std::string> joy_arg("j", "joystick", "path to joystick configuration", false, "string", cmd);

    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    std::vector<std::shared_ptr<Joystick>> joysticks;
    for (const std::string& path : joy_arg.getValue()) {
        auto joy = std::make_shared<Joystick>();
        Error err = joy->load(path);
        if (err) {
            std::cerr << "Error loading joystick " << path << ": " << path << std::endl;
            return 1;
        }

        joysticks.push_back(joy);
    }

    std::filesystem::path out_dir = std::filesystem::absolute(out_arg.getValue());
    if (!std::filesystem::is_directory(out_dir)) {
        std::cerr << "error: output directory specified does not exist or is not a directory" << std::endl;
        return 1;
    }

    app = std::make_unique<App>(out_dir);

    glfwSetErrorCallback(onError);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW!!!\n");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Awesome Demo", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    glfwSetWindowSizeCallback(window, onWindowSize);
    glfwSetKeyCallback(window, onKey);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    // Absolutify our shader paths
    std::filesystem::path vert_path = std::filesystem::absolute(vert_arg.getValue());
    std::filesystem::path frag_path = std::filesystem::absolute(frag_arg.getValue());

    // Setup our app
    std::optional<std::string> err = app->setup(vert_path, frag_path, joysticks);
    if (err.has_value()) {
        std::cerr << "Error initializing app" << std::endl << err.value() << std::endl;
        return 1;
    }

#ifdef BENCHMARK
    double frames = 0;
    float last_benchmark = 0;
#else
    double last_frame = -1;
    double per_frame = 1 / 30.;
#endif
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double t = glfwGetTime();

#ifdef BENCHMARK
        frames++;
        app->draw(window, t);
        if (t - last_benchmark >= 1.0) {
            printf("%f ms/frame\n", 1000.0/double(frames));
            frames = 0;
            last_benchmark = t;
        }
#else
        if (last_frame < 0 || t - last_frame > per_frame) {
            app->draw(window, t);
            last_frame = t;
        }
#endif

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
