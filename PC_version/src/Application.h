#pragma once

#include <array>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "ParticleSystem.h"
#include "Renderer.h"
#include "SceneTypes.h"
#include "SolarSystem.h"
#include "Spacecraft.h"
#include "Starfield.h"
#include "UiOverlay.h"

namespace cosmosim {

class Application {
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool initialize(int width = 1440, int height = 900, const std::string& title = "CosmoSim 3D Professional v10 FINAL POLISH");
    int run();

private:
    GLFWwindow* window_ = nullptr;
    int width_ = 1440;
    int height_ = 900;
    std::string baseTitle_ = "CosmoSim 3D Professional v10 FINAL POLISH";

    SolarSystem solarSystem_;
    SimulationSettings settings_;
    Camera camera_;
    Renderer renderer_;
    ParticleSystem particles_;
    Spacecraft spacecraft_;
    Starfield starfield_;
    UiOverlay ui_;

    int selectedIndex_ = 3;
    bool cursorCaptured_ = true;
    bool firstMouse_ = true;
    bool showHelp_ = false;
    double lastMouseX_ = 0.0;
    double lastMouseY_ = 0.0;
    double lastFrameTime_ = 0.0;
    double titleAccumulator_ = 0.0;
    float fps_ = 0.0f;
    float smoothedFrameTime_ = 1.0f / 60.0f;
    std::array<bool, GLFW_KEY_LAST + 1> previousKeys_{};

    bool fullscreen_ = false;
    int windowedX_ = 120;
    int windowedY_ = 80;
    int windowedWidth_ = 1440;
    int windowedHeight_ = 900;

    void processContinuousInput(float deltaTime);
    void processDiscreteInput();
    bool keyPressed(int key);
    void selectBody(int index, bool frame = false);
    void cycleCameraMode();
    void launchMission();
    void setCursorCaptured(bool captured);
    void updateWindowTitle(double deltaTime);
    void toggleFullscreen();
    int pickBody(double mouseX, double mouseY) const;

    static Application* fromWindow(GLFWwindow* window);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition);
    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

} // namespace cosmosim
