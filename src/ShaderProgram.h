#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <filesystem>
#include <map>

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

    private:
        std::map<GLenum, Shader> shaders_;
        bool should_switch_ = false;
        ProgramHandle internal_program_;
        ProgramHandle external_program_;
};

#endif
