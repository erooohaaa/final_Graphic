#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "App.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace {

struct InputState {
    App* app = nullptr;
    bool orbiting = false;
    double lastX = 0.0;
    double lastY = 0.0;
};

std::filesystem::path executableDirectory(const char* argument) {
#ifdef _WIN32
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length > 0 && length < buffer.size()) {
        return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
    }
#elif defined(__linux__)
    std::error_code error;
    const auto executable = std::filesystem::read_symlink("/proc/self/exe", error);
    if (!error) return executable.parent_path();
#endif
    return std::filesystem::absolute(argument).parent_path();
}

std::filesystem::path bestScorePath() {
#ifdef _WIN32
    if (const wchar_t* base = _wgetenv(L"LOCALAPPDATA")) {
        return std::filesystem::path(base) / "AITUBlockBlast3D" / "best.txt";
    }
#else
    if (const char* base = std::getenv("XDG_DATA_HOME")) {
        return std::filesystem::path(base) / "AITUBlockBlast3D" / "best.txt";
    }
    if (const char* base = std::getenv("HOME")) {
        return std::filesystem::path(base) / ".local/share/AITUBlockBlast3D/best.txt";
    }
#endif
    return {};
}

void pointerPosition(GLFWwindow* window, double x, double y) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (!input || !input->app) return;
    int width = 0, height = 0, pixelsX = 0, pixelsY = 0;
    glfwGetWindowSize(window, &width, &height);
    glfwGetFramebufferSize(window, &pixelsX, &pixelsY);
    if (width > 0 && height > 0 && pixelsX > 0 && pixelsY > 0) {
        input->app->resize(pixelsX, pixelsY);
        input->app->pointerMove(static_cast<float>(x * pixelsX / width),
                                static_cast<float>(y * pixelsY / height));
    }
}

void cursorCallback(GLFWwindow* window, double x, double y) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (!input || !input->app) return;
    if (input->orbiting) {
        input->app->orbit(static_cast<float>(x - input->lastX), static_cast<float>(y - input->lastY));
    }
    input->lastX = x;
    input->lastY = y;
    pointerPosition(window, x, y);
}

void mouseCallback(GLFWwindow* window, int button, int action, int) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (!input || !input->app) return;
    glfwGetCursorPos(window, &input->lastX, &input->lastY);
    pointerPosition(window, input->lastX, input->lastY);
    if (button == GLFW_MOUSE_BUTTON_RIGHT) input->orbiting = action == GLFW_PRESS;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) input->app->pointerDown();
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) input->app->pointerUp();
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (!input || !input->app) return;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3) input->app->select(key - GLFW_KEY_1);
    if (key == GLFW_KEY_ESCAPE) input->app->cancel();
    if (key == GLFW_KEY_N) input->app->newGame();
    if (key == GLFW_KEY_F2) input->app->startDemo();
    if (key == GLFW_KEY_R) input->app->resetCamera();
    if (key == GLFW_KEY_L) input->app->toggleLightMotion();
}

void scrollCallback(GLFWwindow* window, double, double amount) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (input && input->app) input->app->zoom(static_cast<float>(amount));
}

void focusCallback(GLFWwindow* window, int focused) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (!focused && input && input->app) {
        input->orbiting = false;
        input->app->cancel();
    }
}

void screenshot(const std::filesystem::path& destination, int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::ofstream file(destination, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot write screenshot: " + destination.string());
    file << "P6\n" << width << ' ' << height << "\n255\n";
    for (int y = height - 1; y >= 0; --y) {
        file.write(reinterpret_cast<const char*>(pixels.data() + static_cast<std::size_t>(y) * width * 3),
                   static_cast<std::streamsize>(width) * 3);
    }
    if (!file) throw std::runtime_error("Screenshot write failed");
}

}

int main(int argc, char** argv) {
    GLFWwindow* window = nullptr;
    bool initialized = false;
    try {
        bool demo = false;
        bool smokeTest = false;
        std::filesystem::path capture;
        for (int i = 1; i < argc; ++i) {
            const std::string argument(argv[i]);
            if (argument == "--demo") demo = true;
            else if (argument == "--smoke-test") smokeTest = true;
            else if (argument == "--screenshot" && i + 1 < argc) capture = argv[++i];
            else throw std::runtime_error("Usage: BlockBlast3D [--demo] [--smoke-test] [--screenshot image.ppm]");
        }
        glfwSetErrorCallback([](int code, const char* message) {
            std::cerr << "GLFW " << code << ": " << message << '\n';
        });
        if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");
        initialized = true;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_SAMPLES, 4);
        window = glfwCreateWindow(1280, 800, "Block Blast 3D | AITU SE-2407", nullptr, nullptr);
        if (!window) throw std::runtime_error("Cannot create an OpenGL 3.3 window. Check the GPU driver.");
        glfwSetWindowSizeLimits(window, 960, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwMakeContextCurrent(window);
        if (!gladLoadGL(glfwGetProcAddress) || !GLAD_GL_VERSION_3_3) {
            throw std::runtime_error("OpenGL 3.3 is not available");
        }
        glfwSwapInterval(1);
        glEnable(GL_MULTISAMPLE);
        std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n'
                  << "GPU: " << glGetString(GL_RENDERER) << '\n'
                  << "LMB: select/place or drag | RMB: orbit | Wheel: zoom\n"
                  << "1-3: piece | Esc: cancel | N: new | F2: demo | R: reset view\n"
                  << "L: animate light | Arrows + PgUp/PgDn: move light\n";
        {
            const auto seed = static_cast<std::uint32_t>(
                std::chrono::system_clock::now().time_since_epoch().count());
            App app(executableDirectory(argv[0]) / "assets/shaders",
                    smokeTest ? std::filesystem::path{} : bestScorePath(), seed);
            InputState input;
            input.app = &app;
            glfwGetCursorPos(window, &input.lastX, &input.lastY);
            glfwSetWindowUserPointer(window, &input);
            glfwSetCursorPosCallback(window, cursorCallback);
            glfwSetMouseButtonCallback(window, mouseCallback);
            glfwSetKeyCallback(window, keyCallback);
            glfwSetScrollCallback(window, scrollCallback);
            glfwSetWindowFocusCallback(window, focusCallback);
            if (demo) app.startDemo();
            double previous = glfwGetTime();
            int frames = 0;
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                const double now = glfwGetTime();
                const float elapsed = static_cast<float>(now - previous);
                const float delta = std::min(elapsed, 0.1f);
                previous = now;
                int width = 0, height = 0;
                glfwGetFramebufferSize(window, &width, &height);
                if (width <= 0 || height <= 0) {
                    glfwWaitEventsTimeout(0.05);
                    continue;
                }
                app.resize(width, height);
                double cursorX = 0.0, cursorY = 0.0;
                glfwGetCursorPos(window, &cursorX, &cursorY);
                pointerPosition(window, cursorX, cursorY);
                if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
                    const auto held = [window](int key) { return glfwGetKey(window, key) == GLFW_PRESS ? 1.0f : 0.0f; };
                    const float dx = held(GLFW_KEY_RIGHT) - held(GLFW_KEY_LEFT);
                    const float dy = held(GLFW_KEY_PAGE_UP) - held(GLFW_KEY_PAGE_DOWN);
                    const float dz = held(GLFW_KEY_DOWN) - held(GLFW_KEY_UP);
                    if (dx != 0.0f || dy != 0.0f || dz != 0.0f) app.moveLight(dx, dy, dz, delta);
                }
                app.update(elapsed);
                app.render();
                ++frames;
                if (!capture.empty() && frames == 8) screenshot(capture, width, height);
                glfwSwapBuffers(window);
                if ((smokeTest || !capture.empty()) && frames >= 8) break;
            }
            glfwSetWindowUserPointer(window, nullptr);
        }
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        if (window) glfwDestroyWindow(window);
        if (initialized) glfwTerminate();
        return 1;
    }
}
