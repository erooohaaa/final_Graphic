#include "Hud.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>

namespace {

using Glyph = std::array<unsigned char, 7>;

Glyph glyph(unsigned char ch) {
    if (ch >= 'a' && ch <= 'z') {
        ch = static_cast<unsigned char>(ch - 'a' + 'A');
    }
    switch (ch) {
    case 'A': return {14, 17, 17, 31, 17, 17, 17};
    case 'B': return {30, 17, 17, 30, 17, 17, 30};
    case 'C': return {14, 17, 16, 16, 16, 17, 14};
    case 'D': return {30, 17, 17, 17, 17, 17, 30};
    case 'E': return {31, 16, 16, 30, 16, 16, 31};
    case 'F': return {31, 16, 16, 30, 16, 16, 16};
    case 'G': return {14, 17, 16, 23, 17, 17, 15};
    case 'H': return {17, 17, 17, 31, 17, 17, 17};
    case 'I': return {14, 4, 4, 4, 4, 4, 14};
    case 'J': return {7, 2, 2, 2, 18, 18, 12};
    case 'K': return {17, 18, 20, 24, 20, 18, 17};
    case 'L': return {16, 16, 16, 16, 16, 16, 31};
    case 'M': return {17, 27, 21, 21, 17, 17, 17};
    case 'N': return {17, 25, 21, 19, 17, 17, 17};
    case 'O': return {14, 17, 17, 17, 17, 17, 14};
    case 'P': return {30, 17, 17, 30, 16, 16, 16};
    case 'Q': return {14, 17, 17, 17, 21, 18, 13};
    case 'R': return {30, 17, 17, 30, 20, 18, 17};
    case 'S': return {15, 16, 16, 14, 1, 1, 30};
    case 'T': return {31, 4, 4, 4, 4, 4, 4};
    case 'U': return {17, 17, 17, 17, 17, 17, 14};
    case 'V': return {17, 17, 17, 17, 17, 10, 4};
    case 'W': return {17, 17, 17, 21, 21, 27, 17};
    case 'X': return {17, 17, 10, 4, 10, 17, 17};
    case 'Y': return {17, 17, 10, 4, 4, 4, 4};
    case 'Z': return {31, 1, 2, 4, 8, 16, 31};
    case '0': return {14, 17, 19, 21, 25, 17, 14};
    case '1': return {4, 12, 4, 4, 4, 4, 14};
    case '2': return {14, 17, 1, 2, 4, 8, 31};
    case '3': return {30, 1, 1, 14, 1, 1, 30};
    case '4': return {2, 6, 10, 18, 31, 2, 2};
    case '5': return {31, 16, 16, 30, 1, 1, 30};
    case '6': return {14, 16, 16, 30, 17, 17, 14};
    case '7': return {31, 1, 2, 4, 8, 8, 8};
    case '8': return {14, 17, 17, 14, 17, 17, 14};
    case '9': return {14, 17, 17, 15, 1, 1, 14};
    case ' ': return {0, 0, 0, 0, 0, 0, 0};
    case '.': return {0, 0, 0, 0, 0, 6, 6};
    case ',': return {0, 0, 0, 0, 6, 6, 4};
    case ':': return {0, 6, 6, 0, 6, 6, 0};
    case ';': return {0, 6, 6, 0, 6, 6, 4};
    case '!': return {4, 4, 4, 4, 4, 0, 4};
    case '?': return {14, 17, 1, 2, 4, 0, 4};
    case '+': return {0, 4, 4, 31, 4, 4, 0};
    case '-': return {0, 0, 0, 31, 0, 0, 0};
    case '/': return {1, 2, 2, 4, 8, 8, 16};
    case '\\': return {16, 8, 8, 4, 2, 2, 1};
    case '(': return {2, 4, 8, 8, 8, 4, 2};
    case ')': return {8, 4, 2, 2, 2, 4, 8};
    case '[': return {14, 8, 8, 8, 8, 8, 14};
    case ']': return {14, 2, 2, 2, 2, 2, 14};
    case '{': return {3, 4, 4, 8, 4, 4, 3};
    case '}': return {24, 4, 4, 2, 4, 4, 24};
    case '=': return {0, 0, 31, 0, 31, 0, 0};
    case '#': return {10, 10, 31, 10, 31, 10, 10};
    case '%': return {25, 25, 2, 4, 8, 19, 19};
    case '<': return {1, 2, 4, 8, 4, 2, 1};
    case '>': return {16, 8, 4, 2, 4, 8, 16};
    case '_': return {0, 0, 0, 0, 0, 0, 31};
    case '|': return {4, 4, 4, 4, 4, 4, 4};
    case '\'': return {4, 4, 8, 0, 0, 0, 0};
    case '"': return {10, 10, 10, 0, 0, 0, 0};
    case '*': return {0, 21, 14, 31, 14, 21, 0};
    case '^': return {4, 10, 17, 0, 0, 0, 0};
    case '~': return {0, 0, 9, 22, 0, 0, 0};
    case '`': return {8, 4, 2, 0, 0, 0, 0};
    case '$': return {4, 15, 20, 14, 5, 30, 4};
    case '&': return {12, 18, 20, 8, 21, 18, 13};
    case '@': return {14, 17, 23, 21, 23, 16, 14};
    default: return {14, 17, 1, 2, 4, 0, 4};
    }
}

}

Hud::Hud(const std::filesystem::path& shaderDir)
    : shader_(shaderDir / "hud.vert", shaderDir / "hud.frag") {
    vertices_.reserve(65536);
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, r)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Hud::~Hud() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}

void Hud::begin(int width, int height) {
    width_ = std::max(1, width);
    height_ = std::max(1, height);
    vertices_.clear();
}

void Hud::triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color) {
    vertices_.push_back({a.x, a.y, color.r, color.g, color.b, color.a});
    vertices_.push_back({b.x, b.y, color.r, color.g, color.b, color.a});
    vertices_.push_back({c.x, c.y, color.r, color.g, color.b, color.a});
}

void Hud::rect(float x, float y, float width, float height, glm::vec4 color) {
    if (width <= 0.0f || height <= 0.0f || color.a <= 0.0f) {
        return;
    }
    triangle({x, y}, {x + width, y}, {x + width, y + height}, color);
    triangle({x, y}, {x + width, y + height}, {x, y + height}, color);
}

void Hud::roundedRect(float x, float y, float width, float height, float radius, glm::vec4 color) {
    if (width <= 0.0f || height <= 0.0f || color.a <= 0.0f) {
        return;
    }
    radius = std::clamp(radius, 0.0f, std::min(width, height) * 0.5f);
    if (radius <= 0.0f) {
        rect(x, y, width, height, color);
        return;
    }
    constexpr float pi = 3.14159265358979323846f;
    constexpr int segments = 8;
    std::array<glm::vec2, (segments + 1) * 4> perimeter{};
    const std::array<glm::vec2, 4> centers = {
        glm::vec2{x + radius, y + radius},
        glm::vec2{x + width - radius, y + radius},
        glm::vec2{x + width - radius, y + height - radius},
        glm::vec2{x + radius, y + height - radius}
    };
    std::size_t index = 0;
    for (int corner = 0; corner < 4; ++corner) {
        const float start = pi + static_cast<float>(corner) * pi * 0.5f;
        for (int segment = 0; segment <= segments; ++segment) {
            const float angle = start + static_cast<float>(segment) / segments * pi * 0.5f;
            perimeter[index++] = centers[corner] + radius * glm::vec2(std::cos(angle), std::sin(angle));
        }
    }
    const glm::vec2 center{x + width * 0.5f, y + height * 0.5f};
    for (std::size_t i = 0; i < perimeter.size(); ++i) {
        triangle(center, perimeter[i], perimeter[(i + 1) % perimeter.size()], color);
    }
}

void Hud::text(const std::string& value, float x, float y, float scale, glm::vec4 color) {
    if (scale <= 0.0f || color.a <= 0.0f) {
        return;
    }
    const float startX = x;
    for (const unsigned char ch : value) {
        if (ch == '\n') {
            x = startX;
            y += 9.0f * scale;
            continue;
        }
        if (ch == '\r') {
            continue;
        }
        if (ch == '\t') {
            x += 24.0f * scale;
            continue;
        }
        const Glyph rows = glyph(ch);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if ((rows[row] & (1u << (4 - col))) != 0) {
                    rect(x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
        x += 6.0f * scale;
    }
}

float Hud::textWidth(const std::string& value, float scale) const {
    if (scale <= 0.0f) {
        return 0.0f;
    }
    float longest = 0.0f;
    float current = 0.0f;
    for (const char ch : value) {
        if (ch == '\n') {
            longest = std::max(longest, current);
            current = 0.0f;
        } else if (ch == '\t') {
            current += 24.0f;
        } else if (ch != '\r') {
            current += 6.0f;
        }
    }
    longest = std::max(longest, current);
    return longest > 0.0f ? (longest - 1.0f) * scale : 0.0f;
}

void Hud::flush() {
    if (vertices_.empty()) {
        return;
    }
    const GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
    const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    GLboolean depthMask = GL_TRUE;
    GLint oldProgram = 0;
    GLint oldVao = 0;
    GLint oldBuffer = 0;
    GLint srcRgb = 0;
    GLint dstRgb = 0;
    GLint srcAlpha = 0;
    GLint dstAlpha = 0;
    GLint rgbEquation = 0;
    GLint alphaEquation = 0;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldBuffer);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &rgbEquation);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &alphaEquation);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    shader_.use();
    shader_.setMat4("uProjection", glm::ortho(0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f));
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices_.size() * sizeof(Vertex)), vertices_.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
    glBindVertexArray(static_cast<GLuint>(oldVao));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(oldBuffer));
    glUseProgram(static_cast<GLuint>(oldProgram));
    glBlendEquationSeparate(rgbEquation, alphaEquation);
    glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
    glDepthMask(depthMask);
    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
    if (cullWasEnabled) glEnable(GL_CULL_FACE);
    if (!blendWasEnabled) glDisable(GL_BLEND);
    vertices_.clear();
}
