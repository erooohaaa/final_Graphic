#pragma once
#include <glm/glm.hpp>
#include <vector>

// All coordinates are world-space; no dependencies on the camera or window.
enum class Location { Park, Beach, Winter };
enum class MeshShape { Box, Sphere, Cylinder, Cone };
enum class Surface { Plastic, Wood, Stone, Grass, Sand, Water, Snow, Leaves, Metal };
struct SceneObject {
    MeshShape shape = MeshShape::Box;
    glm::vec3 center{0};
    glm::vec3 size{1};
    glm::vec3 color{1};
    Surface material = Surface::Plastic;
    glm::vec3 rotation{0};
    float emission = 0;
    float alpha = 1;
    bool castsShadow = true;
};
class SunPosition {
public:
    void move(float horizontal, float vertical, float dt);
    void reset();
    glm::vec3 direction() const;
    float azimuthDegrees() const;
    float elevationDegrees() const;
private:
    float azimuth_ = -0.80f;
    float elevation_ = 0.70f;
};
class Environment {
public:
    static const char* name(Location location);
    static glm::vec3 skyColor(Location location);
    static std::vector<SceneObject> build(Location location, bool highQuality);
    static std::vector<SceneObject> animated(Location location, float time, bool highQuality);
};
