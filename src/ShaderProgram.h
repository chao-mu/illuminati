#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <filesystem>
#include <map>
#include <functional>
#include <vector>

#include <GL/glew.h>

#include "Result.h"

class ShaderProgram {
    public:
        using ShaderHandle = GLuint;
        using ProgramHandle = GLuint;

        struct Shader {
            ShaderHandle handle;
            std::filesystem::file_time_type last_modified;
            std::filesystem::path path;
            GLenum type;
        };

        ShaderProgram();
        ~ShaderProgram();

        Error loadShader(GLenum type, const std::string& path);
        Error update();
        ShaderHandle getProgram();

        std::optional<GLint> getUniformLoc(std::string name);
        void setUniform(const std::string& name, std::function<void(GLint&)> f);
        void markUniformInUse(const std::string& name);
        std::vector<std::string> getUnsetUniforms();

    private:
        std::map<GLenum, Shader> shaders_;
        std::map<std::string, GLint> uniforms_;
        std::vector<GLint> set_uniforms_;
        bool should_switch_ = false;
        ProgramHandle program_;
};

#endif
