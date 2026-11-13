#pragma once

#include "Shader.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Hud {
public:
    explicit Hud(const std::filesystem::path& shaderDir);
    ~Hud();

    Hud(const Hud&) = delete;
    Hud& operator=(const Hud&) = delete;

    void begin(int width, int height);
    void rect(float x, float y, float width, float height, glm::vec4 color);
    void roundedRect(float x, float y, float width, float height, float radius, glm::vec4 color);
    void text(const std::string& value, float x, float y, float scale, glm::vec4 color);
    float textWidth(const std::string& value, float scale) const;
    void flush();

private:
    struct Vertex {
        float x;
        float y;
        float r;
        float g;
        float b;
        float a;
    };

    void triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color);

    Shader shader_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    int width_ = 1;
    int height_ = 1;
    std::vector<Vertex> vertices_;
};
