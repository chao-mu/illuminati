#include "Image.h"

#include "lodepng.h"

Error Image::setup(std::filesystem::path& path, GLenum texture_unit) {
    std::vector<unsigned char> image;
    unsigned int errc = lodepng::decode(image, width_, height_, path);
    if (errc != 0) {
        return "PNG decoder error " + std::to_string(errc) + ": "+ lodepng_error_text(errc);
    }

    std::vector<unsigned char> flipped;
    for (int row = height_ - 1; row >= 0; row--) {
        int offset = row * (width_ * 4);
        flipped.insert(flipped.end(), image.begin() + offset, image.begin() + offset + (width_ * 4));
    }

    if (tex_id_ == GL_FALSE) {
        glGenTextures(1, &tex_id_);
    }

    texture_unit_ = texture_unit;

    // Store the previous active texture so we can revert to it
    GLint prev_active = 0;
    glGetIntegeri_v(GL_ACTIVE_TEXTURE, 1, &prev_active);

    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, &flipped[0]);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(prev_active);

    initialized_ = true;

    return {};
}

GLuint Image::getID() const {
    return tex_id_;
}

GLenum Image::getTextureUnit() const {
    return texture_unit_;
}

unsigned int Image::getWidth() const {
    return width_;
}

unsigned int Image::getHeight() const {
    return height_;
}

bool Image::isInitialized() const {
    return initialized_;
}
