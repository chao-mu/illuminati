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
#include "MathUtil.h"

#define IMG_UNIT 0
#define IMG_UNIT_GL GL_TEXTURE0

#define WEBCAM_UNIT 1
#define WEBCAM_UNIT_GL GL_TEXTURE1

#define LAST_OUTPUT_UNIT 2
#define LAST_OUTPUT_UNIT_GL GL_TEXTURE2

#define SRC 0
#define DEST 1

App::App(const std::filesystem::path& out_dir, std::pair<int, int> size)
    : img_(new Image()), out_dir_(out_dir), size_(size)  {}


Error App::screenshot() {
    std::stringstream s;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::time_t now = std::time(nullptr);
    s << "output-" << std::put_time(std::localtime(&now), "%Y-%m-%d_") << ms << ".png";
    std::filesystem::path dest = out_dir_ / s.str();

    int width = size_.first;
    int height = size_.second;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    GLsizei pixels = width * height;
    std::vector<unsigned char> image(pixels * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

    // Flip upside down (PNG's coordinate system is upside down to OpenGL's)
    std::vector<unsigned char> flipped;
    for (int row = height - 1; row >= 0; row--) {
        int offset = row * (width * 4);
        flipped.insert(flipped.end(), image.begin() + offset, image.begin() + offset + (width * 4));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned errc = lodepng::encode(dest, flipped, width, height);
    if (errc) {
        return "encoder error " + std::to_string(errc) + ": "+ lodepng_error_text(errc);
    }

    return {};
}

bool App::setupWebcam(int dev) {
    if (!webcam_) {
        webcam_ = std::make_unique<Webcam>(dev);
    }

    Error err = webcam_->startIf();
    if (err) {
        std::cerr << "Error opening webcam: " << err.value() << std::endl;
        return false;
    }

    return true;
}

std::optional<std::string> App::setup(
        std::filesystem::path vert_path,
        std::filesystem::path frag_path,
        std::vector<std::shared_ptr<Joystick>> joysticks,
        std::filesystem::path& img_path
        ) {
    joy_manager_ = std::make_unique<JoystickManager>();
    joysticks_ = joysticks;
    for (auto& joy : joysticks) {
        joy_manager_->addJoystick(joy);
    }

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

    if (img_path != "") {
        Error err = img_->setup(img_path, IMG_UNIT);
        if (err) {
            return err;
        }
    }

    // Bind vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Copy indices into element buffer
    glGenBuffers(1, &ebo);
    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Copy position vetex attributes
    GLfloat pos[] = {
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    glGenBuffers(1, &pos_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // Copy texture coord vertex attributes
    GLfloat coords[] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
    };
    glGenBuffers(1, &coord_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, coord_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    // Unbind our friends
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Texture creation
    glGenTextures(1, &webcam_tex_);
    glBindTexture(GL_TEXTURE_2D, webcam_tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(2, output_texs_);
    for (const auto& id : output_texs_) {
        glBindTexture(GL_TEXTURE_2D, id);

        // Give an empty image to OpenGL ( the last "0" )
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_.first, size_.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glFramebufferTexture(GL_FRAMEBUFFER, draw_bufs_[SRC], output_texs_[SRC], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, draw_bufs_[DEST], output_texs_[DEST], 0);

    // This comment is a reminder of what we didn't unbind
    // glBindVertexArray(0);

    return {};
}

void App::draw(GLFWwindow* window, double t) {
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);

    joy_manager_->update();
    for (auto& joy : joysticks_) {
        joy->update(t);
    }

    std::string err = program_->update().value_or("");
    if (err != "") {
        if (err != last_err_) {
            std::cerr << err << std::endl;
            last_err_ = err;
        }
    } else if (last_err_ != "") {
        std::cerr << "- No More Errors! - " << std::endl;
        last_err_ = "";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glDrawBuffer(draw_bufs_[DEST]);

    // Use our shader
    GLuint program = program_->getProgram();
    glUseProgram(program);

    if (img_->isInitialized()) {
        program_->setUniform("img0", [this, program](GLint& id) {
            glActiveTexture(IMG_UNIT_GL);
            glBindTexture(GL_TEXTURE_2D, img_->getID());
            glUniform1i(id, 0);
        });

        program_->setUniform("iResolutionImg0", [this, program](GLint& id) {
            glProgramUniform2f(program, id, (float)img_->getWidth(), (float)img_->getHeight());
        });
    }

    // Read webcam
    std::optional<GLint> webcam_loc = program_->getUniformLoc("cap0");
    if ((program_->getUniformLoc("iResolutionCap0") || webcam_loc) && setupWebcam(0)) {
        cv::Mat frame;
        if (webcam_->read(frame) && webcam_loc) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            flip(frame, frame, -1);

            cv::Size size = frame.size();

            glActiveTexture(WEBCAM_UNIT_GL);
            glBindTexture(GL_TEXTURE_2D, webcam_tex_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.width, size.height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);
            glUniform1i(webcam_loc.value(), WEBCAM_UNIT);
        }

        program_->markUniformInUse("cap0");
        program_->setUniform("iResolutionCap0", [this, program](GLint& id) {
            glProgramUniform2f(program, id, (GLfloat) webcam_->getWidth(), (GLfloat) webcam_->getHeight());
        });
    }

    program_->setUniform("iResolution", [this, program](GLint& id) {
        glProgramUniform2f(program, id, (float)size_.first, (float)size_.second);
    });

    program_->setUniform("iTime", [t, program](GLint& id) {
        glProgramUniform1f(program, id, (float)t);
    });

    program_->setUniform("lastOut", [this, program](GLint& id) {
        glActiveTexture(LAST_OUTPUT_UNIT_GL);
        glBindTexture(GL_TEXTURE_2D, output_texs_[SRC]);
        glUniform1i(id, LAST_OUTPUT_UNIT);
    });

    program_->setUniform("firstPass", [this, program](GLint& id) {
        glProgramUniform1i(program, id, first_pass_);
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

    glViewport(0,0, size_.first, size_.second);

    // Draw our vertices
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Swap the ping pong buffer!
    std::swap(draw_bufs_[SRC], draw_bufs_[DEST]);
    std::swap(output_texs_[SRC], output_texs_[DEST]);

    // Calculate blit settings
    DrawInfo draw_info = DrawInfo::scaleCenter(
            static_cast<float>(size_.first),
            static_cast<float>(size_.second),
            static_cast<float>(win_width),
            static_cast<float>(win_height));

    // Draw to the screen
    glDrawBuffer(GL_BACK);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glReadBuffer(draw_bufs_[SRC]);
    glViewport(0,0, win_width, win_height);
    glBlitFramebuffer(
        0,0, size_.first, size_.second,
        draw_info.x0, draw_info.y0, draw_info.x1, draw_info.y1,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_NEAREST
    );

    std::string warning;
    for (const auto& unset : program_->getUnsetUniforms()) {
        warning += "WARNING: unset in-use uniform '" + unset + "'\n";
    }
    if (warning != "") {
        if (warning != last_warning_) {
            // There's a newline at the end of warning
            std::cerr << warning << std::flush;
            last_warning_ = warning;
        }
    } else if (last_warning_ != "") {
        std::cerr << "- No More Warnings! - " << std::endl;
        last_warning_ = "";
    }

    first_pass_ = false;
}

void App::onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_P:
                auto err = screenshot();
                if (err) std::cerr << "Error screenshotting: " << err.value() << std::endl;
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

#undef WEBCAM_UNIT
#undef IMG_UNIT
#undef LAST_OUTPUT_UNIT
#undef SRC
#undef DEST
