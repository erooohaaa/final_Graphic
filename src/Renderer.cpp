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

void triangle(std::vector<Vertex>& v,Vertex a,Vertex b,Vertex c) {
    glm::vec3 normal=a.normal+b.normal+c.normal;
    if(glm::dot(glm::cross(b.position-a.position,c.position-a.position),normal)<0)std::swap(b,c);
    if(glm::length(glm::cross(b.position-a.position,c.position-a.position))>0.000001f) {
        v.push_back(a);v.push_back(b);v.push_back(c);
    }
}
std::vector<Vertex> sphere() {
    std::vector<Vertex> v;constexpr int rings=12,slices=20;
    auto point=[](int j,int i) {
        float u=float(i)/slices,t=float(j)/rings;
        float a=u*6.283185307f,b=t*3.14159265f;
        glm::vec3 n(std::sin(b)*std::cos(a),std::cos(b),std::sin(b)*std::sin(a));
        return Vertex{n*0.5f,n,{u,t}};
    };
    for(int j=0;j<rings;++j)for(int i=0;i<slices;++i) {
        auto a=point(j,i),b=point(j+1,i),c=point(j+1,i+1),d=point(j,i+1);
        triangle(v,a,b,c);triangle(v,a,c,d);
    }
    return v;
}
std::vector<Vertex> roundShape(bool cone) {
    std::vector<Vertex> v;constexpr int slices=24;
    for(int i=0;i<slices;++i) {
        float a=i*6.283185307f/slices,b=(i+1)*6.283185307f/slices;
        glm::vec3 na=glm::normalize(glm::vec3(std::cos(a),cone?0.5f:0.0f,std::sin(a)));
        glm::vec3 nb=glm::normalize(glm::vec3(std::cos(b),cone?0.5f:0.0f,std::sin(b)));
        glm::vec3 pa(0.5f*std::cos(a),-0.5f,0.5f*std::sin(a));
        glm::vec3 pb(0.5f*std::cos(b),-0.5f,0.5f*std::sin(b));
        glm::vec3 qa=cone?glm::vec3(0,0.5f,0):pa+glm::vec3(0,1,0);
        glm::vec3 qb=cone?qa:pb+glm::vec3(0,1,0);
        triangle(v,{pa,na,{float(i)/slices,0}},{pb,nb,{float(i+1)/slices,0}},{qb,nb,{float(i+1)/slices,1}});
        if(!cone)triangle(v,{pa,na,{0,0}},{qb,nb,{1,1}},{qa,na,{0,1}});
        triangle(v,{{0,-0.5f,0},{0,-1,0},{0.5f,0.5f}},{pb,{0,-1,0},{1,0}},{pa,{0,-1,0},{0,0}});
        if(!cone)triangle(v,{{0,0.5f,0},{0,1,0},{0.5f,0.5f}},{qa,{0,1,0},{0,0}},{qb,{0,1,0},{1,0}});
    }
    return v;
}

}

Renderer::Renderer(const std::filesystem::path& dir)
    :shader_(dir/"world.vert",dir/"world.frag"),
     shadowShader_(dir/"shadow.vert",dir/"shadow.frag"),
     skyShader_(dir/"sky.vert",dir/"sky.frag") {
    std::vector<Vertex> vertices;
    std::array<std::vector<Vertex>,4> meshes{makeBeveledCube(),sphere(),roundShape(false),roundShape(true)};
    for(int i=0;i<4;++i) {
        ranges_[i]={static_cast<GLint>(vertices.size()),static_cast<GLsizei>(meshes[i].size())};
        vertices.insert(vertices.end(),meshes[i].begin(),meshes[i].end());
    }
    glGenVertexArrays(1,&vao_);glGenBuffers(1,&vbo_);
    if(!vao_||!vbo_)throw std::runtime_error("Cannot allocate scene mesh");
    glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,vbo_);
    glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,position)));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,normal)));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,uv)));
    glBindVertexArray(0);glBindBuffer(GL_ARRAY_BUFFER,0);
    glGenFramebuffers(1,&shadowFbo_);glGenTextures(1,&shadowTexture_);
    if(!shadowFbo_||!shadowTexture_)throw std::runtime_error("Cannot allocate shadow resources");
    allocateShadow(2048);
}
Renderer::~Renderer() {
    if(shadowTexture_)glDeleteTextures(1,&shadowTexture_);
    if(shadowFbo_)glDeleteFramebuffers(1,&shadowFbo_);
    if(vbo_)glDeleteBuffers(1,&vbo_);
    if(vao_)glDeleteVertexArrays(1,&vao_);
}
void Renderer::allocateShadow(int size) {
    if(shadowSize_==size)return;
    GLint previous=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&previous);
    glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,shadowTexture_);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,size,size,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
    float border[]={1,1,1,1};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
    glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,shadowTexture_,0);
    glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Sun shadow framebuffer incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER,static_cast<GLuint>(previous));
    glBindTexture(GL_TEXTURE_2D,0);glActiveTexture(GL_TEXTURE0);shadowSize_=size;
}
void Renderer::configure(glm::vec3 sky,float time,bool shadows,bool highQuality) {
    sky_=sky;time_=time;shadows_=shadows;highQuality_=highQuality;
}
void Renderer::begin(const glm::mat4& view,const glm::mat4& projection,glm::vec3 eye,glm::vec3 directionToSun) {
    view_=view;projection_=projection;eye_=eye;
    sun_=glm::length(directionToSun)>0.001f?glm::normalize(directionToSun):glm::vec3(0,1,0);
    commands_.clear();
}
void Renderer::draw(const SceneObject& o) {
    if(o.size.x<=0||o.size.y<=0||o.size.z<=0)return;
    glm::mat4 model=glm::translate(glm::mat4(1),o.center);
    model=glm::rotate(model,glm::radians(o.rotation.y),glm::vec3(0,1,0));
    model=glm::rotate(model,glm::radians(o.rotation.x),glm::vec3(1,0,0));
    model=glm::rotate(model,glm::radians(o.rotation.z),glm::vec3(0,0,1));
    model=glm::scale(model,o.size);commands_.push_back({o,model});
}
void Renderer::box(glm::vec3 center,glm::vec3 size,glm::vec3 color,float textureAmount,float emission,float alpha) {
    (void)textureAmount;
    SceneObject o;o.center=center;o.size=size;o.color=color;o.emission=emission;o.alpha=alpha;
    o.castsShadow=alpha>=0.99f;draw(o);
}
void Renderer::issue(const Command& c,Shader& shader,bool depthOnly) {
    shader.setMat4("uModel",c.model);
    if(!depthOnly) {
        shader.setVec3("uColor",c.object.color);
        shader.setInt("uMaterial",static_cast<int>(c.object.material));
        shader.setFloat("uEmission",c.object.emission);shader.setFloat("uAlpha",c.object.alpha);
    }
    const auto& range=ranges_[static_cast<int>(c.object.shape)];
    glDrawArrays(GL_TRIANGLES,range.start,range.count);
}
void Renderer::end(bool worldScene) {
    GLint viewport[4],framebuffer;glGetIntegerv(GL_VIEWPORT,viewport);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&framebuffer);
    GLboolean scissor=glIsEnabled(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LESS);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);glFrontFace(GL_CCW);
    glDisable(GL_BLEND);glDepthMask(GL_TRUE);glBindVertexArray(vao_);
    glm::vec3 up=std::abs(sun_.y)>0.97f?glm::vec3(0,0,1):glm::vec3(0,1,0);
    lightMatrix_=glm::ortho(-27.0f,27.0f,-27.0f,27.0f,1.0f,110.0f)*glm::lookAt(sun_*52.0f,glm::vec3(0),up);
    bool shadowPass=worldScene&&shadows_;
    if(shadowPass) {
        allocateShadow(highQuality_?2048:1024);
        glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,0);glActiveTexture(GL_TEXTURE0);
        glDisable(GL_SCISSOR_TEST);glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo_);
        glViewport(0,0,shadowSize_,shadowSize_);glClear(GL_DEPTH_BUFFER_BIT);
        shadowShader_.use();shadowShader_.setMat4("uLightMatrix",lightMatrix_);
        glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(1.5f,3.0f);
        for(const auto& c:commands_)if(c.object.castsShadow&&c.object.alpha>=0.99f&&c.object.emission<1.0f)issue(c,shadowShader_,true);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER,static_cast<GLuint>(framebuffer));
        glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
        if(scissor)glEnable(GL_SCISSOR_TEST);
    }
    if(worldScene) {
        glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);glDisable(GL_CULL_FACE);
        skyShader_.use();skyShader_.setMat4("uInverseViewProjection",glm::inverse(projection_*view_));
        skyShader_.setVec3("uEye",eye_);skyShader_.setVec3("uSunDirection",sun_);
        skyShader_.setVec3("uSkyColor",sky_);skyShader_.setFloat("uTime",time_);
        glDrawArrays(GL_TRIANGLES,0,3);
        glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glEnable(GL_CULL_FACE);
    }
    shader_.use();shader_.setMat4("uView",view_);shader_.setMat4("uProjection",projection_);
    shader_.setMat4("uLightMatrix",lightMatrix_);shader_.setVec3("uEye",eye_);
    shader_.setVec3("uSunDirection",sun_);shader_.setVec3("uSkyColor",sky_);
    shader_.setFloat("uTime",time_);shader_.setInt("uShadows",shadowPass?1:0);
    shader_.setInt("uWorldScene",worldScene?1:0);shader_.setInt("uShadowMap",1);
    glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,shadowTexture_);glActiveTexture(GL_TEXTURE0);
    for(const auto& c:commands_)if(c.object.alpha>=0.99f)issue(c,shader_,false);
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
    for(const auto& c:commands_)if(c.object.alpha<0.99f)issue(c,shader_,false);
    glDepthMask(GL_TRUE);glDisable(GL_BLEND);glBindVertexArray(0);
}
