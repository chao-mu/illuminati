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

        std::ostringstream err;
        err << "Error compiling " << path <<  ":\n" << std::string(begin(v), end(v));
        return err.str();
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

void ShaderProgram::setUniform(const std::string& name, std::function<void(GLint&)> f) {
    if (!uniforms_.count(name)) {
        return;
    }

    GLint id = uniforms_.at(name);
    f(id);

    set_uniforms_.push_back(id);
}

std::vector<std::string> ShaderProgram::getUnsetUniforms() {
    std::vector<std::string> unused;
    for (const auto& kv : uniforms_) {
        std::string name = kv.first;
        GLint id = kv.second;

        if (set_uniforms_.end() == std::find(set_uniforms_.begin(), set_uniforms_.end(), id)) {
            unused.push_back(name);
        }
    }

    return unused;
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
            std::ostringstream err;
            err << "Error linking shader program: \n" << std::string(begin(v), end(v));
            return err.str();
        }

        GLint uni_name_len = 0;
        glGetProgramiv(internal_program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uni_name_len);

        GLint count;
        glGetProgramiv(internal_program_, GL_ACTIVE_UNIFORMS, &count);
        uniforms_.clear();
        for (GLuint i = 0; i < (GLuint)count; i++)
        {
            std::vector<GLchar> name(uni_name_len);

            GLsizei length;
            GLint size;
            GLenum type;
            glGetActiveUniform(internal_program_, i, uni_name_len, &length, &size, &type, &name[0]);

            GLint loc = glGetUniformLocation(internal_program_, name.data());
            uniforms_[std::string(name.data())] = loc;
        }

        // Elevate internal program
        glDeleteProgram(external_program_);
        external_program_ = internal_program_;
        internal_program_ = glCreateProgram();

        should_switch_ = false;
    }

    set_uniforms_.clear();

    return {};
}

std::optional<GLint> ShaderProgram::getUniformLoc(std::string name) {
    if (!uniforms_.count(name)) {
        return {};
    }

    return uniforms_.at(name);
}

GLuint ShaderProgram::getProgram() {
    return external_program_;
}
#undef MAX_UNIFORM_NAME_LEN
