#include "App.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

bool ScreenRect::contains(float px, float py) const {
    return px >= x && py >= y && px < x + w && py < y + h;
}

App::App(const std::filesystem::path& shaderDir, const std::filesystem::path& bestPath,
    std::uint32_t seed)
    : game_(seed), renderer_(shaderDir), hud_(shaderDir), bestPath_(bestPath), seed_(seed) {
    rebuildScenery();
    readBest();
    resize(width_, height_);
}

void App::resize(int width, int height) {
    width_ = std::max(width, 1);
    height_ = std::max(height, 1);
    scale_ = std::min(width_ / 1280.0f, height_ / 800.0f);
    float margin = 28.0f * scale_;
    float panelWidth = 300.0f * scale_;
    float sideX = width_ - margin - panelWidth;
    boardRect_ = {margin, 126.0f * scale_, sideX - 2.0f * margin,
        height_ - 214.0f * scale_};
    for (int i = 0; i < 3; ++i) {
        cards_[i] = {sideX, (170.0f + i * 132.0f) * scale_, panelWidth, 116.0f * scale_};
    }
    newRect_ = {sideX, 590.0f * scale_, panelWidth, 44.0f * scale_};
    demoRect_ = {sideX, 648.0f * scale_, panelWidth, 36.0f * scale_};
    overlayRestart_ = {boardRect_.x + boardRect_.w * 0.5f - 120.0f * scale_,
        boardRect_.y + boardRect_.h * 0.5f + 65.0f * scale_, 240.0f * scale_, 44.0f * scale_};
    updateHover();
}

glm::vec3 App::blockColor(int color) {
    const glm::vec3 palette[] = {
        {0.21f, 0.29f, 0.40f}, {1.00f, 0.54f, 0.20f}, {0.31f, 0.64f, 1.00f},
        {0.21f, 0.88f, 0.82f}, {0.73f, 0.43f, 1.00f}, {0.98f, 0.36f, 0.48f},
        {0.51f, 0.86f, 0.34f}, {1.00f, 0.78f, 0.25f}
    };
    return palette[std::clamp(color, 0, 7)];
}

void App::readBest() {
    if (bestPath_.empty()) {
        return;
    }
    std::ifstream input(bestPath_);
    long long value = 0;
    if (input >> value && value >= 0 && value <= 2147483647LL) {
        best_ = static_cast<int>(value);
    }
}

void App::saveBest() {
    if (bestPath_.empty() || demo_) {
        return;
    }
    std::error_code error;
    std::filesystem::create_directories(bestPath_.parent_path(), error);
    if (!error) {
        std::ofstream output(bestPath_);
        if (output) {
            output << best_ << '\n';
        }
    }
}

void App::update(float deltaTime) {
    float dt = std::clamp(deltaTime, 0.0f, 0.1f);
    float animationDt=paused_?0.0f:dt;
    time_ += animationDt;
    messageTime_ = std::max(0.0f, messageTime_ - dt);
    fpsTime_ += std::max(deltaTime, 0.0f);
    ++fpsFrames_;
    if (fpsTime_ >= 0.5f) {
        fps_ = static_cast<int>(std::round(fpsFrames_ / fpsTime_));
        fpsFrames_ = 0;
        fpsTime_ = 0.0f;
    }
    for (auto& particle : particles_) {
        particle.life -= animationDt;
        particle.velocity.y -= animationDt * 8.0f;
        particle.position += particle.velocity * animationDt;
    }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
        [](const Particle& particle) { return particle.life <= 0.0f; }), particles_.end());
}

void App::pointerMove(float x, float y) {
    mouseX_ = x;
    mouseY_ = y;
    updateHover();
}

void App::updateHover() {
    hover_ = false;
    if (selected_ < 0 || !game_.pieces()[selected_].available || game_.gameOver() ||
        !boardRect_.contains(mouseX_, mouseY_) || boardRect_.w <= 0 || boardRect_.h <= 0) {
        return;
    }
    float x = 2.0f * (mouseX_ - boardRect_.x) / boardRect_.w - 1.0f;
    float y = 1.0f - 2.0f * (mouseY_ - boardRect_.y) / boardRect_.h;
    glm::mat4 inverse = glm::inverse(camera_.projection(boardRect_.w / boardRect_.h) * camera_.view());
    glm::vec4 nearPoint = inverse * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 farPoint = inverse * glm::vec4(x, y, 1.0f, 1.0f);
    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;
    glm::vec3 direction = glm::normalize(glm::vec3(farPoint - nearPoint));
    if (std::abs(direction.y) < 0.00001f) {
        return;
    }
    float t = (0.12f - nearPoint.y) / direction.y;
    if (t < 0.0f) {
        return;
    }
    glm::vec3 hit = glm::vec3(nearPoint) + t * direction;
    if (hit.x < -4.0f || hit.x >= 4.0f || hit.z < -4.0f || hit.z >= 4.0f) {
        return;
    }
    const auto& piece = game_.pieces()[selected_];
    hoverX_ = static_cast<int>(std::floor(hit.x + 4.0f)) - piece.width() / 2;
    hoverZ_ = static_cast<int>(std::floor(hit.z + 4.0f)) - piece.height() / 2;
    hover_ = true;
}

glm::vec2 App::boardCellScreen(int x, int z) const {
    glm::vec4 clip = camera_.projection(boardRect_.w / boardRect_.h) * camera_.view() *
        glm::vec4(x - 3.5f, 0.12f, z - 3.5f, 1.0f);
    clip /= clip.w;
    return {boardRect_.x + (clip.x + 1.0f) * boardRect_.w * 0.5f,
        boardRect_.y + (1.0f - clip.y) * boardRect_.h * 0.5f};
}

void App::select(int slot) {
    if (!paused_ && !help_ && slot >= 0 && slot < 3 && game_.pieces()[slot].available && !game_.gameOver()) {
        selected_ = slot;
        message_ = "CLICK A CELL OR DRAG THE PIECE";
        messageTime_ = 0.0f;
        updateHover();
    }
}

void App::pointerDown() {
    if(paused_||help_)return;
    if (newRect_.contains(mouseX_, mouseY_) ||
        (game_.gameOver() && overlayRestart_.contains(mouseX_, mouseY_))) {
        newGame();
        return;
    }
    if (demoRect_.contains(mouseX_, mouseY_)) {
        startDemo();
        return;
    }
    if (game_.gameOver()) {
        return;
    }
    for (int slot = 0; slot < 3; ++slot) {
        if (cards_[slot].contains(mouseX_, mouseY_) && game_.pieces()[slot].available) {
            select(slot);
            dragging_ = true;
            return;
        }
    }
    if (selected_ >= 0 && hover_) {
        placeSelected();
    }
}

void App::pointerUp() {
    if(paused_||help_){dragging_=false;return;}
    if (dragging_ && hover_) {
        placeSelected();
    }
    dragging_ = false;
}

void App::cancel() {
    selected_ = -1;
    dragging_ = false;
    hover_ = false;
}

void App::placeSelected() {
    if (!hover_ || selected_ < 0) {
        return;
    }
    auto result = game_.place(selected_, hoverX_, hoverZ_);
    if (!result.placed) {
        message_ = "DOES NOT FIT - TRY ANOTHER CELL";
        messageColor_ = {1.0f, 0.42f, 0.46f, 1.0f};
        messageTime_ = 1.8f;
        return;
    }
    for (const auto& clear : result.cleared) {
        int index = clear.cell.x + clear.cell.z * puzzle::BoardSize;
        particles_.push_back({{clear.cell.x - 3.5f, 0.48f, clear.cell.z - 3.5f},
            {static_cast<float>(index % 3 - 1) * 0.35f, 2.2f,
                static_cast<float>((index / 3) % 3 - 1) * 0.35f},
            blockColor(clear.color), 0.6f});
    }
    if (result.lines > 0) {
        message_ = std::to_string(result.lines) + (result.lines == 1 ? " LINE" : " LINES") +
            "  +" + std::to_string(result.gained) + "  COMBO X" + std::to_string(game_.combo());
        messageColor_ = {0.35f, 0.95f, 0.79f, 1.0f};
    } else {
        message_ = "+" + std::to_string(result.gained);
        messageColor_ = {0.88f, 0.93f, 1.0f, 1.0f};
    }
    messageTime_ = 2.0f;
    if (!demo_ && game_.score() > best_) {
        best_ = game_.score();
        saveBest();
    }
    cancel();
}

void App::newGame() {
    paused_=false;
    game_ = puzzle::Game(++seed_);
    demo_ = false;
    particles_.clear();
    cancel();
    message_ = "SELECT A PIECE TO START";
    messageTime_ = 0.0f;
}

void App::startDemo() {
    paused_=false;
    game_.startDemo();
    demo_ = true;
    particles_.clear();
    cancel();
    select(0);
    message_ = "DEMO: FILL THE TWO EMPTY CELLS IN THE FRONT ROW";
    messageColor_ = {1.0f, 0.79f, 0.35f, 1.0f};
    messageTime_ = 8.0f;
}

void App::orbit(float dx, float dy) {
    camera_.orbit(dx, dy);
    updateHover();
}

void App::zoom(float amount) {
    camera_.zoom(amount);
    updateHover();
}

void App::resetCamera() {
    camera_.reset();
    updateHover();
}

void App::moveLight(float dx, float dy, float dz, float deltaTime) {
    if (dx == 0.0f && dy == 0.0f && dz == 0.0f) {
        return;
    }
    // Input changes spherical angles, never camera-space positions.
    sun_.move(dx,dy-dz,deltaTime);
}

void App::toggleLightMotion() { sun_.reset(); }
void App::cycleLocation() {
    location_=static_cast<Location>((static_cast<int>(location_)+1)%3);
    rebuildScenery(); // Does not reset puzzle or change the sun's world direction.
}
void App::togglePause() { paused_=!paused_;cancel(); }
void App::toggleShadows() { shadows_=!shadows_; }
void App::toggleQuality() { highQuality_=!highQuality_;rebuildScenery(); }
void App::toggleHelp() { help_=!help_;cancel(); }
glm::vec3 App::sunDirection() const {return sun_.direction();}
Location App::location() const {return location_;}
bool App::shadowsEnabled() const {return shadows_;}
void App::rebuildScenery() {scenery_=Environment::build(location_,highQuality_);}

const puzzle::Game& App::game() const {
    return game_;
}

ScreenRect App::boardViewport() const {
    return boardRect_;
}

void App::drawBoard() {
    glViewport(static_cast<int>(boardRect_.x),
        height_ - static_cast<int>(boardRect_.y + boardRect_.h),
        static_cast<int>(boardRect_.w), static_cast<int>(boardRect_.h));
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    renderer_.configure(Environment::skyColor(location_),time_,shadows_,highQuality_);
    renderer_.begin(camera_.view(), camera_.projection(boardRect_.w / boardRect_.h),
        camera_.position(), sun_.direction());
    for(const auto& object:scenery_)renderer_.draw(object);
    for(const auto& object:Environment::animated(location_,time_,highQuality_))renderer_.draw(object);
    renderer_.box({0.0f, -0.35f, 0.0f}, {8.65f, 0.60f, 8.65f}, {0.16f, 0.24f, 0.36f}, 0.35f);
    for (int z = 0; z < puzzle::BoardSize; ++z) {
        for (int x = 0; x < puzzle::BoardSize; ++x) {
            glm::vec3 position(x - 3.5f, 0.04f, z - 3.5f);
            glm::vec3 tile = (x + z) % 2 == 0 ? glm::vec3(0.25f, 0.34f, 0.46f)
                                             : glm::vec3(0.22f, 0.30f, 0.41f);
            renderer_.box(position, {0.94f, 0.16f, 0.94f}, tile, 0.35f);
            int color = game_.board()[z][x];
            if (color != 0) {
                renderer_.box({position.x, 0.43f, position.z}, {0.88f, 0.62f, 0.88f},
                    blockColor(color), 0.12f);
            }
        }
    }

    for (const auto& particle : particles_) {
        float size = 0.8f * std::max(particle.life / 0.6f, 0.05f);
        renderer_.box(particle.position, glm::vec3(size), particle.color, 0.08f, 0.25f);
    }
    if (hover_ && selected_ >= 0) {
        bool valid = game_.canPlace(selected_, hoverX_, hoverZ_);
        glm::vec3 color = valid ? glm::vec3(0.20f, 1.0f, 0.70f) : glm::vec3(1.0f, 0.18f, 0.28f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (const auto& cell : game_.pieces()[selected_].cells) {
            renderer_.box({hoverX_ + cell.x - 3.5f, 0.58f, hoverZ_ + cell.z - 3.5f},
                {0.90f, 0.70f, 0.90f}, color, 0.0f, 0.45f, 0.62f);
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    renderer_.end(true);
}

void App::drawInterface() {
    glViewport(0, 0, width_, height_);
    float s = scale_;
    float sideX = cards_[0].x;
    glm::vec4 white(0.91f, 0.95f, 1.0f, 1.0f);
    glm::vec4 muted(0.48f, 0.59f, 0.73f, 1.0f);
    glm::vec4 cyan(0.28f, 0.91f, 0.80f, 1.0f);
    hud_.begin(width_, height_);
    hud_.text("BLOCK BLAST", 28 * s, 28 * s, 4.0f * s, white);
    hud_.text("3D", 310 * s, 35 * s, 2.5f * s, cyan);
    hud_.text(std::string(Environment::name(location_))+" [T]   F1 HELP", 30 * s, 72 * s, 1.3f * s, muted);
    hud_.text(demo_ ? "DEMO SCORE" : "SCORE", sideX, 25 * s, 1.3f * s, muted);
    hud_.text(std::to_string(game_.score()), sideX, 48 * s, 3.4f * s, white);
    hud_.text("BEST", sideX + 183 * s, 25 * s, 1.3f * s, muted);
    hud_.text(std::to_string(best_), sideX + 183 * s, 48 * s, 3.0f * s, {1.0f, 0.79f, 0.35f, 1.0f});
    hud_.rect(28 * s, 103 * s, width_ - 56 * s, 1.0f * s, {0.18f, 0.25f, 0.35f, 1.0f});
    hud_.rect(boardRect_.x,boardRect_.y,boardRect_.w,22*s,{0.03f,0.06f,0.09f,0.78f});
    hud_.text(paused_?"PAUSED [P]":"THE BOARD", (boardRect_.x+10*s), (boardRect_.y+7*s), 1.3f * s, muted);
    hud_.text("YOUR PIECES", sideX, 130 * s, 1.7f * s, white);
    if (game_.combo() > 0) {
        hud_.text("COMBO X" + std::to_string(game_.combo()), (boardRect_.x+180*s), (boardRect_.y+7*s), 1.3f * s, cyan);
    }
    for (int slot = 0; slot < 3; ++slot) {
        auto card = cards_[slot];
        bool available = game_.pieces()[slot].available;
        bool selected = available && slot == selected_;
        bool hovered = card.contains(mouseX_, mouseY_);
        glm::vec4 border = selected ? cyan : glm::vec4(0.20f, 0.28f, 0.40f, 1.0f);
        hud_.roundedRect(card.x, card.y, card.w, card.h, 12 * s, border);
        hud_.roundedRect(card.x + 2 * s, card.y + 2 * s, card.w - 4 * s, card.h - 4 * s,
            10 * s, hovered ? glm::vec4(0.12f, 0.18f, 0.28f, 1.0f) : glm::vec4(0.09f, 0.14f, 0.23f, 1.0f));
        hud_.text(std::to_string(slot + 1), card.x + 15 * s, card.y + 13 * s, 1.8f * s,
            available ? white : muted);
        std::string label = !available ? "PLACED" : selected ? "SELECTED" : "DRAG OR CLICK";
        hud_.text(label, card.x + 48 * s, card.y + 17 * s, 1.2f * s, selected ? cyan : muted);
        if (!available) {
            hud_.text("DONE", card.x + 125 * s, card.y + 65 * s, 1.8f * s, muted);
        }
    }
    hud_.text("USE ALL 3 TO GET NEW PIECES", sideX + 5 * s, 566 * s, 1.15f * s, muted);
    bool hoverNew = newRect_.contains(mouseX_, mouseY_);
    hud_.roundedRect(newRect_.x, newRect_.y, newRect_.w, newRect_.h, 9 * s,
        hoverNew ? glm::vec4(0.27f, 0.88f, 0.75f, 1.0f) : glm::vec4(0.19f, 0.74f, 0.64f, 1.0f));
    hud_.text("NEW GAME [N]", newRect_.x + 73 * s, newRect_.y + 15 * s, 1.7f * s,
        {0.035f, 0.09f, 0.14f, 1.0f});
    hud_.roundedRect(demoRect_.x, demoRect_.y, demoRect_.w, demoRect_.h, 8 * s,
        {0.13f, 0.19f, 0.29f, 1.0f});
    hud_.text("DEMO BOARD [F2]", demoRect_.x + 73 * s, demoRect_.y + 12 * s, 1.4f * s, white);
    hud_.text("SHADOWS: " + std::string(shadows_ ? "ON [B]" : "OFF [B]"), sideX + 5 * s, 711 * s, 1.2f * s, muted);
    hud_.text(std::to_string(fps_) + " FPS", sideX + 224 * s, 711 * s, 1.2f * s, muted);
    std::string status = messageTime_ > 0.0f ? message_ : selected_ >= 0 ?
        "CLICK TO PLACE   ESC TO CANCEL" : "CHOOSE A PIECE FROM THE RIGHT";
    float statusScale = 1.4f * s;
    float maxWidth = boardRect_.w - 20 * s;
    if (hud_.textWidth(status, statusScale) > maxWidth) {
        statusScale = maxWidth / hud_.textWidth(status, 1.0f);
    }
    hud_.text(status, boardRect_.x + 10 * s, height_ - 73 * s, statusScale,
        messageTime_ > 0.0f ? messageColor_ : muted);
    hud_.rect(28 * s, height_ - 46 * s, width_ - 56 * s, 1 * s, {0.18f, 0.25f, 0.35f, 1.0f});
    hud_.text("T SCENE   P PAUSE   B SHADOWS   Q QUALITY   ARROWS SUN   L RESET SUN   F1 HELP",
        30 * s, height_ - 29 * s, 1.15f * s, muted);
    std::string sunText="SUN AZ "+std::to_string(static_cast<int>(sun_.azimuthDegrees()))+
        "  EL "+std::to_string(static_cast<int>(sun_.elevationDegrees()))+
        (highQuality_?"   HIGH [Q]":"   LOW [Q]");
    hud_.text(sunText,sideX+5*s,736*s,1.05f*s,muted);
    hud_.flush();
}

void App::drawPieces() {
    for (int slot = 0; slot < 3; ++slot) {
        const auto& piece = game_.pieces()[slot];
        if (!piece.available) {
            continue;
        }
        const auto& card = cards_[slot];
        int x = static_cast<int>(card.x + 25 * scale_);
        int y = height_ - static_cast<int>(card.y + card.h - 8 * scale_);
        int width = std::max(1, static_cast<int>(card.w - 50 * scale_));
        int height = std::max(1, static_cast<int>(card.h - 40 * scale_));
        glViewport(x, y, width, height);
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glm::vec3 eye(0.0f, 9.0f, 6.0f);
        glm::mat4 view = glm::lookAt(eye, glm::vec3(0), glm::vec3(0, 1, 0));
        glm::vec3 extent((piece.width() - 1.0f + 0.87f) * 0.5f, 0.26f,
            (piece.height() - 1.0f + 0.87f) * 0.5f);
        float projectedWidth = 0.0f;
        float projectedHeight = 0.0f;
        for (int cx : {-1, 1}) {
            for (int cy : {-1, 1}) {
                for (int cz : {-1, 1}) {
                    glm::vec4 corner = view * glm::vec4(extent * glm::vec3(cx, cy, cz), 1.0f);
                    projectedWidth = std::max(projectedWidth, std::abs(corner.x));
                    projectedHeight = std::max(projectedHeight, std::abs(corner.y));
                }
            }
        }
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        float halfWidth = std::max({3.65f, projectedWidth * 1.12f, projectedHeight * aspect * 1.12f});
        float halfHeight = halfWidth / aspect;
        renderer_.begin(view,
            glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 50.0f),
            eye, {-3.0f, 8.0f, 5.0f});
        for (const auto& cell : piece.cells) {
            renderer_.box({cell.x - (piece.width() - 1) * 0.5f, 0.0f,
                cell.z - (piece.height() - 1) * 0.5f}, {0.87f, 0.52f, 0.87f},
                blockColor(piece.color), 0.12f, 0.12f);
        }
        renderer_.end(false);
        glDisable(GL_SCISSOR_TEST);
    }
}

void App::drawGameOver() {
    if (!game_.gameOver()) {
        return;
    }
    float s = scale_;
    glViewport(0, 0, width_, height_);
    hud_.begin(width_, height_);
    hud_.rect(boardRect_.x, boardRect_.y, boardRect_.w, boardRect_.h, {0.02f, 0.04f, 0.08f, 0.78f});
    float cx = boardRect_.x + boardRect_.w * 0.5f;
    float cy = boardRect_.y + boardRect_.h * 0.5f;
    hud_.roundedRect(cx - 205 * s, cy - 115 * s, 410 * s, 245 * s, 14 * s, {0.09f, 0.14f, 0.23f, 1.0f});
    hud_.text("GAME OVER", cx - 132 * s, cy - 74 * s, 3.3f * s, {0.96f, 0.97f, 1.0f, 1.0f});
    hud_.text("NO REMAINING PIECE FITS", cx - 137 * s, cy - 24 * s, 1.4f * s, {0.61f, 0.70f, 0.81f, 1.0f});
    std::string score = "SCORE " + std::to_string(game_.score());
    hud_.text(score, cx - hud_.textWidth(score, 2 * s) * 0.5f, cy + 14 * s, 2 * s, {1.0f, 0.78f, 0.30f, 1.0f});
    hud_.roundedRect(overlayRestart_.x, overlayRestart_.y, overlayRestart_.w, overlayRestart_.h,
        8 * s, {0.19f, 0.77f, 0.64f, 1.0f});
    hud_.text("PLAY AGAIN", overlayRestart_.x + 62 * s, overlayRestart_.y + 15 * s, 1.7f * s,
        {0.04f, 0.10f, 0.15f, 1.0f});
    hud_.flush();
}

void App::render() {
    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE);
    glViewport(0, 0, width_, height_);
    glClearColor(0.045f, 0.07f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateHover();
    drawBoard();
    drawInterface();
    drawPieces();
    drawGameOver();
    drawHelp();
    glViewport(0, 0, width_, height_);
}

void App::drawHelp() {
    if(!help_)return;
    glViewport(0,0,width_,height_);hud_.begin(width_,height_);
    hud_.rect(0,0,float(width_),float(height_),{0.02f,0.04f,0.07f,0.96f});
    const char* lines[]={"BLOCK BLAST 3D - OUTDOOR EDITION", "F1 / H CLOSE HELP",
        "LMB PLACE / DRAG    1-3 CHOOSE    ESC CANCEL", "RMB ORBIT    WHEEL ZOOM    R RESET CAMERA",
        "T PARK / BEACH / WINTER    P PAUSE", "B SUN SHADOWS ON/OFF    Q HIGH / LOW QUALITY",
        "LEFT / RIGHT SUN AZIMUTH    UP / DOWN SUN HEIGHT", "PAGE UP / DOWN ALSO CHANGE SUN HEIGHT    L RESET SUN",
        "SUN STAYS FIXED WHEN THE CAMERA MOVES", "N NEW GAME    F2 DEMO BOARD",
        "MARAT YERKANAT / AYAGAN BAGDAT", "ELEUSINOV DANIYAL / DIAS MADYSHEV"};
    for(int i=0;i<12;++i)hud_.text(lines[i],65*scale_,(58+i*48)*scale_,1.65f*scale_,{0.9f,0.95f,1,1});
    hud_.flush();
}
