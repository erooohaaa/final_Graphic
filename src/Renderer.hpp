#pragma once
#include "Environment.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include <filesystem>
#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>

class Renderer {
public:
    explicit Renderer(const std::filesystem::path& shaderDirectory);
    ~Renderer();
    Renderer(const Renderer&)=delete;
    Renderer& operator=(const Renderer&)=delete;
    void configure(glm::vec3 sky,float time,bool shadows,bool highQuality);
    void begin(const glm::mat4& view,const glm::mat4& projection,glm::vec3 eye,glm::vec3 directionToSun);
    void draw(const SceneObject& object);
    void box(glm::vec3 center,glm::vec3 size,glm::vec3 color,float textureAmount=0.15f,
        float emission=0.0f,float alpha=1.0f);
    void end(bool worldScene=true);
private:
    struct Command { SceneObject object; glm::mat4 model; };
    struct Range { GLint start=0; GLsizei count=0; };
    Shader shader_,shadowShader_,skyShader_;
    Texture texture_;
    GLuint vao_=0,vbo_=0,shadowFbo_=0,shadowTexture_=0;
    Range ranges_[4];
    std::vector<Command> commands_;
    glm::mat4 view_{1},projection_{1},lightMatrix_{1};
    glm::vec3 eye_{0},sun_{0,1,0},sky_{0.3f,0.5f,0.8f};
    float time_=0;
    bool shadows_=true,highQuality_=true;
    int shadowSize_=0;
    void allocateShadow(int size);
    void issue(const Command& command,Shader& shader,bool depthOnly);
};
