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

std::optional<std::pair<int, int>> parseRes(const std::string& s) {
    size_t x = s.find("x");
    if (x == std::string::npos) {
        return {};
    }

    int w, h;
    try {
       w = std::stoi(s.substr(0, x));
       h = std::stoi(s.substr(x + 1));
    } catch (std::invalid_argument& e) {
        return {};
    }

    return std::make_pair(w, h);
}

int main(int argc, char** argv) {
    TCLAP::CmdLine cmd("Illuminati - Everything is Light");

    TCLAP::ValueArg<std::string> vert_arg("", "vert", "path to vertex shader", false, "vert.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> frag_arg("", "frag", "path to fragment shader", false, "frag.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> out_arg("", "out-dir", "path to output directory", false, ".", "string", cmd);
    TCLAP::MultiArg<std::string> joy_arg("j", "joystick", "path to joystick configuration", false, "string", cmd);
    TCLAP::ValueArg<std::string> res_arg("r", "resolution", "Resolution in the format axb where 'a' is with and 'b' is height", false, "1280x720", "string", cmd);
    TCLAP::ValueArg<std::string> window_arg("s", "screen", "Window size in the format axb where 'a' is with and 'b' is height", false, "1280x720", "string", cmd);
    TCLAP::ValueArg<std::string> img_arg("i", "img", "texture image path", false, "", "string", cmd);
    TCLAP::ValueArg<int> loop_arg("l", "loop", "apply shader X times and set iteration uniform", false, 1, "int", cmd);

    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    std::optional<std::pair<int, int>> res_opt = parseRes(res_arg.getValue());
    std::pair<int, int> res;
    if (res_opt) {
        res = res_opt.value();
    } else {
        std::cerr << "error parsing resolution argument, was expecting format axb e.g. 1080x720" << std::endl;
        return 1;
    }

    std::optional<std::pair<int, int>> window_res_opt = parseRes(window_arg.getValue());
    std::pair<int, int> window_res;
    if (window_res_opt) {
        window_res = window_res_opt.value();
    } else {
        std::cerr << "error parsing screen resolution argument, was expecting format axb e.g. 1080x720" << std::endl;
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
    std::filesystem::path img_path = "";
    if (img_arg.isSet()) {
        img_path = std::filesystem::absolute(img_arg.getValue());
        if (!std::filesystem::exists(img_path) || std::filesystem::is_directory(img_path)) {
            std::cerr << "error: specified image path does not exist or is a directory" << std::endl;
            return 1;
        }
    }

    std::filesystem::path out_dir = std::filesystem::absolute(out_arg.getValue());
    if (!std::filesystem::is_directory(out_dir)) {
        std::cerr << "error: output directory specified does not exist or is not a directory" << std::endl;
        return 1;
    }

    app = std::make_unique<App>(out_dir, std::make_pair(res.first, res.second), loop_arg.getValue());

    glfwSetErrorCallback(onError);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW!!!\n");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(window_res.first, window_res.second, "Awesome Demo", NULL, NULL);
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
    std::optional<std::string> err = app->setup(vert_path, frag_path, joysticks, img_path);
    if (err.has_value()) {
        std::cerr << "Error initializing app" << std::endl << err.value() << std::endl;
        return 1;
    }

#ifdef BENCHMARK
    double frames = 0;
    double last_benchmark = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double t = glfwGetTime();

        frames++;
        app->draw(window, t);
        if (t - last_benchmark >= 1.0) {
            printf("%f ms/frame\n", 1000.0/frames);
            frames = 0;
            last_benchmark = t;
        }

        glfwSwapBuffers(window);
    }
#else
    double last_frame = -1;
    double per_frame = 1 / 30.;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double t = glfwGetTime();

        if (last_frame < 0 || t - last_frame > per_frame) {
            app->draw(window, t);
            last_frame = t;
        }

        glfwSwapBuffers(window);
    }
#endif

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
