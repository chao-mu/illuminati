#include "Size.h"

#include <cfloat>
#include <climits>

void Size::set(const std::string& s) {
    size_t x = s.find("x");
    if (x == std::string::npos) {
        throw std::invalid_argument("invalid size format, expected x separating width and height");
    }

    int w, h;
    // May throw an std::invalid_argument
    w = std::stoi(s.substr(0, x));
    h = std::stoi(s.substr(x + 1));

    // Also may throw std::invalid_argument
    set(w, h);
}

template<typename T> void Size::assertValid(T x, const std::string& arg_name) {
    if (x < 0) {
        throw std::invalid_argument(arg_name + " can not be negative");
    }

    if (x > INT_MAX) {
        throw std::invalid_argument(arg_name + " is too large (larger than an int)");
    }

    if (x > FLT_MAX) {
        throw std::invalid_argument(arg_name + " is too large (larger than a float)");
    }
}

void Size::set(int width, int height) {
    assertValid(width, "width");
    assertValid(height, "height");

    width_ = static_cast<unsigned int>(width);
    height_ = static_cast<unsigned int>(height);
}

void Size::set(unsigned int width, unsigned int height) {
    assertValid(width, "width");
    assertValid(height, "height");

    width = width_;
    height = height_;
}
