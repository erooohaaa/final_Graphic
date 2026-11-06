#include "Renderer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

void appendPolygon(std::vector<Vertex>& vertices, const std::vector<glm::vec3>& positions,
                   glm::vec3 normal) {
    normal = glm::normalize(normal);
    const glm::vec3 reference = std::abs(normal.y) > 0.9f
        ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 tangent = glm::normalize(glm::cross(reference, normal));
    const glm::vec3 bitangent = glm::cross(normal, tangent);
    const auto vertex = [&](const glm::vec3& position) {
        return Vertex{position, normal,
            glm::vec2(glm::dot(position, tangent) + 0.5f,
                      glm::dot(position, bitangent) + 0.5f)};
    };
    for (std::size_t i = 1; i + 1 < positions.size(); ++i) {
        glm::vec3 b = positions[i];
        glm::vec3 c = positions[i + 1];
        if (glm::dot(glm::cross(b - positions[0], c - positions[0]), normal) < 0.0f) {
            std::swap(b, c);
        }
        vertices.push_back(vertex(positions[0]));
        vertices.push_back(vertex(b));
        vertices.push_back(vertex(c));
    }
}

std::vector<Vertex> makeBeveledCube() {
    constexpr float outer = 0.5f;
    constexpr float inner = 0.425f;
    const std::array<glm::vec3, 3> axes{
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    std::vector<Vertex> vertices;
    vertices.reserve(132);
    for (int axis = 0; axis < 3; ++axis) {
        const glm::vec3 u = axes[(axis + 1) % 3] * inner;
        const glm::vec3 v = axes[(axis + 2) % 3] * inner;
        for (int sign : {-1, 1}) {
            const glm::vec3 normal = axes[axis] * static_cast<float>(sign);
            const glm::vec3 center = normal * outer;
            appendPolygon(vertices, {center - u - v, center + u - v,
                                     center + u + v, center - u + v}, normal);
        }
    }
    for (int axisA = 0; axisA < 3; ++axisA) {
        for (int axisB = axisA + 1; axisB < 3; ++axisB) {
            const int axisC = 3 - axisA - axisB;
            const glm::vec3 span = axes[axisC] * inner;
            for (int signA : {-1, 1}) {
                for (int signB : {-1, 1}) {
                    const glm::vec3 a = axes[axisA] * static_cast<float>(signA);
                    const glm::vec3 b = axes[axisB] * static_cast<float>(signB);
                    const glm::vec3 first = a * outer + b * inner;
                    const glm::vec3 second = a * inner + b * outer;
                    appendPolygon(vertices, {first - span, second - span,
                                             second + span, first + span}, a + b);
                }
            }
        }
    }
    for (int signX : {-1, 1}) {
        for (int signY : {-1, 1}) {
            for (int signZ : {-1, 1}) {
                const glm::vec3 signs(static_cast<float>(signX), static_cast<float>(signY),
                                      static_cast<float>(signZ));
                appendPolygon(vertices, {
                    signs * glm::vec3(outer, inner, inner),
                    signs * glm::vec3(inner, outer, inner),
                    signs * glm::vec3(inner, inner, outer)
                }, signs);
            }
        }
    }
    return vertices;
}

}

Renderer::Renderer(const std::filesystem::path& shaderDirectory)
    : shader_(shaderDirectory / "world.vert", shaderDirectory / "world.frag") {
    const std::vector<Vertex> vertices = makeBeveledCube();
    vertexCount_ = static_cast<GLsizei>(vertices.size());
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    if (vao_ == 0 || vbo_ == 0) {
        if (vbo_ != 0) {
            glDeleteBuffers(1, &vbo_);
        }
        if (vao_ != 0) {
            glDeleteVertexArrays(1, &vao_);
        }
        throw std::runtime_error("Cannot allocate block mesh buffers.");
    }
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, uv)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Renderer::~Renderer() {
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }
}

void Renderer::begin(const glm::mat4& view, const glm::mat4& projection,
                     glm::vec3 eye, glm::vec3 light) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    shader_.use();
    shader_.setMat4("uView", view);
    shader_.setMat4("uProjection", projection);
    shader_.setVec3("uEye", eye);
    shader_.setVec3("uLight", light);
    shader_.setInt("uTexture", 0);
    texture_.bind(0);
}

void Renderer::box(glm::vec3 center, glm::vec3 size, glm::vec3 color,
                   float textureAmount, float emission, float alpha) {
    if (size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f) {
        return;
    }
    shader_.use();
    texture_.bind(0);
    const glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), center), size);
    shader_.setMat4("uModel", model);
    shader_.setVec3("uColor", color);
    shader_.setFloat("uTextureAmount", std::clamp(textureAmount, 0.0f, 1.0f));
    shader_.setFloat("uEmission", std::max(emission, 0.0f));
    shader_.setFloat("uAlpha", std::clamp(alpha, 0.0f, 1.0f));
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    glBindVertexArray(0);
}
