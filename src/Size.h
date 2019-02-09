#ifndef SIZE_H
#define SIZE_H

#include <string>

class Size {
    public:
        void set(const std::string& width_and_height);
        void set(int width, int height);
        void set(unsigned int width, unsigned int height);

        // TODO: Replace templates with specific per-type functions
        template<typename T> T getWidth() {
            return static_cast<T>(width_);
        }

        template<typename T> T getHeight() {
            return static_cast<T>(height_);
        }

    private:
        template<typename T> void assertValid(T x, const std::string& arg_name);

        unsigned int width_ = 0;
        unsigned int height_ = 0;
};
#endif
