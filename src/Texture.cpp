#include "Texture.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

Texture::Texture() {
    constexpr int side = 128;
    constexpr float pi = 3.14159265358979323846f;
    std::vector<unsigned char> pixels(side * side * 3);
    std::uint32_t state = 732451u;
    for (int y = 0; y < side; ++y) {
        for (int x = 0; x < side; ++x) {
            state = state * 1664525u + 1013904223u;
            const float noise = static_cast<float>((state >> 24u) & 255u) / 255.0f;
            const float grain = std::sin(2.0f * pi * static_cast<float>(x) / 8.0f)
                * std::sin(2.0f * pi * static_cast<float>(y) / 8.0f);
            const int shade = std::clamp(static_cast<int>(229.0f + 12.0f * noise + 5.0f * grain), 0, 255);
            const std::size_t offset = static_cast<std::size_t>((y * side + x) * 3);
            pixels[offset] = static_cast<unsigned char>(shade);
            pixels[offset + 1] = static_cast<unsigned char>(shade);
            pixels[offset + 2] = static_cast<unsigned char>(shade);
        }
    }
    glGenTextures(1, &texture_);
    if (texture_ == 0) {
        throw std::runtime_error("Cannot create the procedural block texture.");
    }
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, side, side, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if (texture_ != 0) {
        glDeleteTextures(1, &texture_);
    }
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_);
}
