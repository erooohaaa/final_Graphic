#pragma once

#include <glad/gl.h>

class Texture {
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(unsigned int unit = 0) const;

private:
    GLuint texture_ = 0;
};
