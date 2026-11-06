#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

#include <filesystem>
#include <glad/gl.h>
#include <glm/glm.hpp>

class Renderer {
public:
    explicit Renderer(const std::filesystem::path& shaderDirectory);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void begin(const glm::mat4& view, const glm::mat4& projection, glm::vec3 eye, glm::vec3 light);
    void box(glm::vec3 center, glm::vec3 size, glm::vec3 color,
             float textureAmount = 0.15f, float emission = 0.0f, float alpha = 1.0f);

private:
    Shader shader_;
    Texture texture_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLsizei vertexCount_ = 0;
};
