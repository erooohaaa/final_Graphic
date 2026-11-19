#include "Shader.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

namespace {

std::string readText(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open shader: " + path.u8string());
    }
    const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (file.bad()) {
        throw std::runtime_error("Cannot read shader: " + path.u8string());
    }
    if (source.empty()) {
        throw std::runtime_error("Shader file is empty: " + path.u8string());
    }
    return source;
}

std::string shaderLog(GLuint shader) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return "No shader compiler log was returned.";
    }
    std::vector<GLchar> log(static_cast<std::size_t>(length));
    GLsizei written = 0;
    glGetShaderInfoLog(shader, length, &written, log.data());
    return std::string(log.data(), static_cast<std::size_t>(written));
}

std::string programLog(GLuint program) {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return "No program linker log was returned.";
    }
    std::vector<GLchar> log(static_cast<std::size_t>(length));
    GLsizei written = 0;
    glGetProgramInfoLog(program, length, &written, log.data());
    return std::string(log.data(), static_cast<std::size_t>(written));
}

GLuint compileShader(GLenum type, const std::string& source, const std::filesystem::path& path) {
    const GLuint shader = glCreateShader(type);
    if (shader == 0) {
        throw std::runtime_error("Cannot create shader: " + path.u8string());
    }
    try {
        const GLchar* text = source.c_str();
        glShaderSource(shader, 1, &text, nullptr);
        glCompileShader(shader);
        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            throw std::runtime_error("Shader compilation failed: " + path.u8string() + "\n" + shaderLog(shader));
        }
    } catch (...) {
        glDeleteShader(shader);
        throw;
    }
    return shader;
}

}

Shader::Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath) {
    const std::string vertexSource = readText(vertexPath);
    const std::string fragmentSource = readText(fragmentPath);
    GLuint vertex = 0;
    GLuint fragment = 0;
    GLuint program = 0;
    try {
        vertex = compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
        fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
        program = glCreateProgram();
        if (program == 0) {
            throw std::runtime_error("Cannot create shader program.");
        }
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            throw std::runtime_error("Shader link failed:\n" + programLog(program));
        }
        glDetachShader(program, vertex);
        glDetachShader(program, fragment);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        program_ = program;
    } catch (...) {
        if (program != 0) {
            glDeleteProgram(program);
        }
        if (vertex != 0) {
            glDeleteShader(vertex);
        }
        if (fragment != 0) {
            glDeleteShader(fragment);
        }
        throw;
    }
}

Shader::~Shader() {
    if (program_ != 0) {
        glDeleteProgram(program_);
    }
}

void Shader::use() const {
    glUseProgram(program_);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program_, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program_, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(program_, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(program_, name.c_str()), value);
}
