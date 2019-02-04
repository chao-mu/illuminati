#include "Result.h"

#include <filesystem>

#include <GL/glew.h>

class Image {
    public:
        Error setup(std::filesystem::path& path, GLenum texture_unit);
        GLuint getID() const;
        unsigned int getWidth() const;
        unsigned int getHeight() const;
        bool isInitialized() const;
        GLenum getTextureUnit() const;

    private:
        GLuint tex_id_ = GL_FALSE;
        GLenum texture_unit_ = GL_TEXTURE0;
        unsigned int width_ = 0;
        unsigned int height_ = 0;
        bool initialized_ = false;
};
