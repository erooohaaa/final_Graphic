#include "OrbitCamera.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 OrbitCamera::position() const {
    float yaw = glm::radians(yaw_);
    float pitch = glm::radians(pitch_);
    return glm::vec3(glm::sin(yaw) * glm::cos(pitch), glm::sin(pitch),
        glm::cos(yaw) * glm::cos(pitch)) * radius_;
}

glm::mat4 OrbitCamera::view() const {
    return glm::lookAt(position(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 OrbitCamera::projection(float aspect) const {
    return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

void OrbitCamera::orbit(float dx, float dy) {
    yaw_ -= dx * 0.3f;
    pitch_ = std::clamp(pitch_ + dy * 0.25f, 30.0f, 80.0f);
}

void OrbitCamera::zoom(float delta) {
    radius_ = std::clamp(radius_ - delta * 0.7f, 10.0f, 22.0f);
}

void OrbitCamera::reset() {
    yaw_ = 0.0f;
    pitch_ = 55.0f;
    radius_ = 13.8f;
}
