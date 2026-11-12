#pragma once

#include <glm/glm.hpp>

class OrbitCamera {
public:
    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    glm::vec3 position() const;
    void orbit(float dx, float dy);
    void zoom(float delta);
    void reset();

private:
    float yaw_ = 0.0f;
    float pitch_ = 38.0f;
    float radius_ = 19.0f;
};
