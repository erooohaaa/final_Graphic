#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "Game.hpp"
#include "Hud.hpp"
#include "OrbitCamera.hpp"
#include "Renderer.hpp"

struct ScreenRect {
    float x;
    float y;
    float w;
    float h;
    bool contains(float px, float py) const;
};

class App {
public:
    App(const std::filesystem::path& shaderDir, const std::filesystem::path& bestPath,
        std::uint32_t seed);
    void resize(int width, int height);
    void update(float deltaTime);
    void render();
    void pointerMove(float x, float y);
    void pointerDown();
    void pointerUp();
    void cancel();
    void select(int slot);
    void orbit(float dx, float dy);
    void zoom(float amount);
    void resetCamera();
    void newGame();
    void startDemo();
    void moveLight(float dx, float dy, float dz, float deltaTime);
    void toggleLightMotion();
    const puzzle::Game& game() const;
    ScreenRect boardViewport() const;
    glm::vec2 boardCellScreen(int x, int z) const;

private:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 color;
        float life;
    };
    puzzle::Game game_;
    Renderer renderer_;
    Hud hud_;
    OrbitCamera camera_;
    std::filesystem::path bestPath_;
    std::uint32_t seed_;
    int width_ = 1280;
    int height_ = 800;
    float scale_ = 1.0f;
    ScreenRect boardRect_{};
    ScreenRect cards_[3]{};
    ScreenRect newRect_{};
    ScreenRect demoRect_{};
    ScreenRect overlayRestart_{};
    int selected_ = -1;
    int hoverX_ = 0;
    int hoverZ_ = 0;
    int best_ = 0;
    bool hover_ = false;
    bool dragging_ = false;
    bool demo_ = false;
    bool animateLight_ = false;
    float mouseX_ = -1.0f;
    float mouseY_ = -1.0f;
    float time_ = 0.0f;
    float messageTime_ = 0.0f;
    float fpsTime_ = 0.0f;
    int fpsFrames_ = 0;
    int fps_ = 0;
    std::string message_ = "SELECT A PIECE TO START";
    glm::vec4 messageColor_{0.56f, 0.69f, 0.81f, 1.0f};
    glm::vec3 light_{-3.0f, 10.0f, 5.0f};
    std::vector<Particle> particles_;
    void updateHover();
    void placeSelected();
    void readBest();
    void saveBest();
    void drawBoard();
    void drawInterface();
    void drawPieces();
    void drawGameOver();
    static glm::vec3 blockColor(int color);
};
