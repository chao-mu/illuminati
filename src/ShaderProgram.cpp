#include "ShaderProgram.h"

#include <fstream>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <iostream>

#include <GLFW/glfw3.h>

ShaderProgram::ShaderProgram() :
    internal_program_(glCreateProgram()), external_program_(glCreateProgram()) {}

ShaderProgram::~ShaderProgram() {
    for (const auto& kv : shaders_) {
        glDeleteShader(kv.second.handle);
    }

    for (const auto& prog : {internal_program_, external_program_}) {
        glDeleteProgram(prog);
    }
}

Error ShaderProgram::loadShader(GLenum type, const std::string& path) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        std::ostringstream err;
        err << "Error loading " << path << " - " <<  std::strerror(errno);
        return err.str();
    }

    std::stringstream stream;
    stream << ifs.rdbuf();
    const std::string source = stream.str();

    const char* c_source = source.c_str();
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &c_source, NULL);
    glCompileShader(shader);

    int status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> v(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, v.data());
        return std::string(begin(v), end(v));
    }

    std::error_code err;
    std::filesystem::file_time_type last_modified = std::filesystem::last_write_time(path, err);
    if (err) {
        return err.message();
    }

    if (shaders_.count(type)) {
        glDeleteShader(shaders_[type].handle);
    }

    shaders_[type] = Shader{};
    Shader& obj = shaders_.at(type);
    obj.handle = shader;
    obj.last_modified = last_modified;
    obj.path = path;
    obj.type = type;

    glAttachShader(internal_program_, shader);

    should_switch_ = true;

    return {};
}

Error ShaderProgram::update() {
    for (auto const kv : shaders_) {
        std::string path = kv.second.path;
        std::error_code errc;
        std::filesystem::file_time_type last_modified = std::filesystem::last_write_time(path, errc);
        if (errc) {
            std::ostringstream s;
            s << "Error accessing " << path << " " << errc.message();
            return s.str();
        }

        if (last_modified > kv.second.last_modified) {
            Error err = loadShader(kv.first, path);
            if (err.has_value()) {
                std::ostringstream s;
                s << "Error loading " << path << ":\n" << err.value();
                return s.str();
            }
        }
    }

    if (should_switch_) {
        glLinkProgram(internal_program_);

        int status = 0;
        glGetProgramiv(internal_program_, GL_LINK_STATUS, &status);
        if (!status) {
            GLint log_length = 0;
            glGetProgramiv(internal_program_, GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> v(log_length);
            glGetProgramInfoLog(internal_program_, log_length, NULL, v.data());
            return std::string(begin(v), end(v));
        }

        // Elevate internal program
        glDeleteProgram(external_program_);
        external_program_ = internal_program_;
        internal_program_ = glCreateProgram();

        should_switch_ = false;
    }

    return {};
}

GLuint ShaderProgram::getProgram() {
    return external_program_;
}
