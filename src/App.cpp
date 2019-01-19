#include "App.h"

#include <iostream>

#include "Result.h"

std::optional<std::string> App::setup() {
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

    // Setup shaders
    program_ = std::make_unique<ShaderProgram>();

    Error err = program_->loadShader(GL_VERTEX_SHADER, "../vert.glsl");
    if (err.has_value()) {
        return err;
    }

    err = program_->loadShader(GL_FRAGMENT_SHADER, "../frag.glsl");
    if (err.has_value()) {
        return err;
    }

    return {};
}

void App::onWindowSize(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0,0, width, height);
}

void App::onError(int /* error */, const char* desc) {
    fputs(desc, stderr);
}

void App::draw(GLFWwindow* window, float t) {
    std::string err = program_->update().value_or("");
    if (err != "") {
        if (err != last_err_) {
            std::cout << err << std::endl;
            last_err_ = err;
        }
    } else {
        last_err_ = "";
    }

    GLuint program = program_->getProgram();
    // Use our shader
    glUseProgram(program);

    // Set uniforms
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    GLint resolutionLoc = glGetUniformLocation(program, "iResolution");
    glProgramUniform2f(program, resolutionLoc, width, height);
    GLint timeLoc = glGetUniformLocation(program, "iTime");
    glProgramUniform1f(program, timeLoc, t);

    // Draw our vertices
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
