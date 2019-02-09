#ifndef IMAGE_H
#define IMAGE_H

#include "Result.h"

#include <filesystem>

#include <GL/glew.h>

#include "Size.h"

class Image {
    public:
        Error setup(std::filesystem::path& path, GLenum texture_unit);
        GLuint getID() const;
        Size getSize() const;
        bool isInitialized() const;
        GLenum getTextureUnit() const;

    private:
        GLuint tex_id_ = GL_FALSE;
        GLenum texture_unit_ = GL_TEXTURE0;
        bool initialized_ = false;
        Size size_;
};
#endif
