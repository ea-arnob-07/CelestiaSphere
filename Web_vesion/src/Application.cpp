#include "Application.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "MathUtils.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
namespace cosmosim {

Application::~Application() {
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

bool Application::initialize(int width, int height, const std::string& title) {
    width_ = width;
    height_ = height;
    baseTitle_ = title;

    glfwSetErrorCallback([](int code, const char* description) {
        std::cerr << "GLFW error " << code << ": " << description << '\n';
    });
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW.\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    window_ = glfwCreateWindow(width_, height_, baseTitle_.c_str(), nullptr, nullptr);
    if (window_ == nullptr) {
        std::cerr << "Could not create OpenGL window.\n";
        return false;
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, cursorPositionCallback);
    glfwSetScrollCallback(window_, scrollCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);

#ifndef __EMSCRIPTEN__
    glewExperimental = GL_TRUE;
    const GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus) << '\n';
        return false;
    }
    glGetError();
#endif

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    settings_.daysPerSecond = 3.0;
    settings_.simulationDay = 0.0;
    settings_.educationalScale = true;
    settings_.showLabels = true;
    solarSystem_.update(0.0, settings_, camera_.position());

    if (!renderer_.initialize(solarSystem_, settings_)) {
        std::cerr << "Renderer initialization failed. Check shader files.\n";
        return false;
    }
    particles_.initialize(2200);
    starfield_.initialize(9000);
    ui_.initialize();

    selectBody(solarSystem_.indexByName("Sun"), false);
    camera_.setMode(CameraMode::Free);
    const float startupRadius = std::clamp(solarSystem_.maximumOrbitRadius(settings_) * 0.62f, 62.0f, 76.0f);
    camera_.setPosition(glm::vec3(0.0f, startupRadius * 0.34f, startupRadius * 1.12f));
    camera_.setYawPitch(-90.0f, -10.0f);
    showHelp_ = true;
#ifdef __EMSCRIPTEN__
    // Browser requires a user gesture (click) before pointer lock is allowed.
    // Cursor capture will be triggered on the first mouse button press.
    setCursorCaptured(false);
#else
    setCursorCaptured(true);
#endif
    lastFrameTime_ = glfwGetTime();
    return true;
}

int Application::run() {
    if (window_ == nullptr) return 1;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg([](void* arg) {
        static_cast<Application*>(arg)->mainLoopStep();
    }, this, 0, true);
#else
    while (glfwWindowShouldClose(window_) == GLFW_FALSE) {
        mainLoopStep();
    }
#endif
    return 0;
}

void Application::mainLoopStep() {
    const double now = glfwGetTime();
    const float deltaTime = static_cast<float>(std::clamp(now - lastFrameTime_, 0.0, 0.1));
    lastFrameTime_ = now;
    smoothedFrameTime_ = glm::mix(smoothedFrameTime_, std::max(deltaTime, 0.00001f), 0.08f);
    fps_ = 1.0f / smoothedFrameTime_;

    glfwPollEvents();
    processContinuousInput(deltaTime);
    processDiscreteInput();

    solarSystem_.update(static_cast<double>(deltaTime), settings_, camera_.position());
    spacecraft_.update(deltaTime, solarSystem_);

    glm::vec3 focusPoint(0.0f);
    float focusRadius = 4.0f;
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(solarSystem_.bodies().size())) {
        const CelestialBody& selected = solarSystem_.bodies()[static_cast<std::size_t>(selectedIndex_)];
        focusPoint = selected.worldPosition;
        focusRadius = selected.renderRadius;
    }
    if (spacecraft_.telemetry().active && camera_.mode() == CameraMode::Cinematic) {
        focusPoint = spacecraft_.telemetry().position;
        focusRadius = 1.5f;
    }
    camera_.update(deltaTime, focusPoint, focusRadius);

    const CelestialBody& sun = solarSystem_.bodies().front();
    particles_.update(deltaTime, sun.worldPosition, sun.renderRadius, spacecraft_.telemetry());

    renderer_.render(
        solarSystem_, camera_, settings_, selectedIndex_, spacecraft_, particles_, starfield_, ui_,
        width_, height_, static_cast<float>(now), fps_, showHelp_);

    updateWindowTitle(static_cast<double>(deltaTime));
    glfwSwapBuffers(window_);
}

void Application::processContinuousInput(float deltaTime) {
    glm::vec3 movement(0.0f);
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) movement.z += 1.0f;
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) movement.z -= 1.0f;
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) movement.x += 1.0f;
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1.0f;
    if (glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS) movement.y += 1.0f;
    if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) movement.y -= 1.0f;
    if (glm::dot(movement, movement) > 0.0f) movement = glm::normalize(movement);
    const bool boost = glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window_, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    camera_.processKeyboard(movement, deltaTime, boost);
}

void Application::processDiscreteInput() {
    if (keyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window_, GLFW_TRUE);
    if (keyPressed(GLFW_KEY_F1)) showHelp_ = !showHelp_;
#ifndef __EMSCRIPTEN__
    // In WebAssembly, fullscreen is handled by the browser/JS natively.
    // Emscripten's GLFW fullscreen implementation conflicts with our CSS.
    if (keyPressed(GLFW_KEY_F11)) toggleFullscreen();
#endif
    if (keyPressed(GLFW_KEY_SPACE)) settings_.paused = !settings_.paused;
    if (keyPressed(GLFW_KEY_TAB)) cycleCameraMode();
    if (keyPressed(GLFW_KEY_F)) camera_.setMode(CameraMode::Follow);
    if (keyPressed(GLFW_KEY_T)) camera_.setMode(CameraMode::TopDown);
    if (keyPressed(GLFW_KEY_C)) camera_.setMode(CameraMode::Cinematic);
    if (keyPressed(GLFW_KEY_V)) camera_.setMode(CameraMode::Free);
    if (keyPressed(GLFW_KEY_R)) {
        solarSystem_.resetEpoch(settings_);
        spacecraft_.abort();
    }
    if (keyPressed(GLFW_KEY_O)) settings_.showOrbits = !settings_.showOrbits;
    if (keyPressed(GLFW_KEY_L)) settings_.showLabels = !settings_.showLabels;
    if (keyPressed(GLFW_KEY_B)) settings_.showAsteroids = !settings_.showAsteroids;
    if (keyPressed(GLFW_KEY_N)) settings_.showAtmospheres = !settings_.showAtmospheres;
    if (keyPressed(GLFW_KEY_G)) settings_.educationalScale = !settings_.educationalScale;
    if (keyPressed(GLFW_KEY_J)) launchMission();
    if (keyPressed(GLFW_KEY_X)) spacecraft_.abort();

    if (keyPressed(GLFW_KEY_MINUS) || keyPressed(GLFW_KEY_KP_SUBTRACT)) {
        settings_.daysPerSecond = std::max(0.05, settings_.daysPerSecond / 2.0);
    }
    if (keyPressed(GLFW_KEY_EQUAL) || keyPressed(GLFW_KEY_KP_ADD)) {
        settings_.daysPerSecond = std::min(4096.0, settings_.daysPerSecond * 2.0);
    }

    if (keyPressed(GLFW_KEY_LEFT)) selectBody(solarSystem_.nextSelectable(selectedIndex_, -1));
    if (keyPressed(GLFW_KEY_RIGHT)) selectBody(solarSystem_.nextSelectable(selectedIndex_, 1));

    const std::array<int, 8> numberKeys = {
        GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4,
        GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8
    };
    const std::array<const char*, 8> planetNames = {
        "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"
    };
    for (std::size_t i = 0; i < numberKeys.size(); ++i) {
        if (keyPressed(numberKeys[i])) selectBody(solarSystem_.indexByName(planetNames[i]));
    }
    if (keyPressed(GLFW_KEY_0)) selectBody(solarSystem_.indexByName("Sun"));
    if (keyPressed(GLFW_KEY_9)) selectBody(solarSystem_.indexByName("Pluto"));
    if (keyPressed(GLFW_KEY_P)) selectBody(solarSystem_.indexByName("Pluto"));
    if (keyPressed(GLFW_KEY_M)) selectBody(solarSystem_.indexByName("Moon"));
    if (keyPressed(GLFW_KEY_H)) selectBody(solarSystem_.indexByName("Black Hole"));
}

bool Application::keyPressed(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    const bool down = glfwGetKey(window_, key) == GLFW_PRESS;
    const bool pressed = down && !previousKeys_[static_cast<std::size_t>(key)];
    previousKeys_[static_cast<std::size_t>(key)] = down;
    return pressed;
}

void Application::selectBody(int index, bool frame) {
    if (index < 0 || index >= static_cast<int>(solarSystem_.bodies().size())) return;
    selectedIndex_ = index;
    if (frame) {
        const CelestialBody& selected = solarSystem_.bodies()[static_cast<std::size_t>(selectedIndex_)];
        camera_.frameTarget(selected.worldPosition, selected.renderRadius);
    }
}

void Application::cycleCameraMode() {
    switch (camera_.mode()) {
        case CameraMode::Free: camera_.setMode(CameraMode::Follow); break;
        case CameraMode::Follow: camera_.setMode(CameraMode::TopDown); break;
        case CameraMode::TopDown: camera_.setMode(CameraMode::Cinematic); break;
        case CameraMode::Cinematic: camera_.setMode(CameraMode::Free); break;
    }
}

void Application::launchMission() {
    const int earth = solarSystem_.indexByName("Earth");
    int target = selectedIndex_;
    if (target == earth || target < 0 || target == solarSystem_.indexByName("Sun")) {
        target = solarSystem_.indexByName("Mars");
        selectedIndex_ = target;
    }
    spacecraft_.launch(earth, target, solarSystem_);
}

void Application::setCursorCaptured(bool captured) {
    cursorCaptured_ = captured;
    firstMouse_ = true;
    glfwSetInputMode(window_, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Application::updateWindowTitle(double deltaTime) {
    titleAccumulator_ += deltaTime;
    if (titleAccumulator_ < 0.25) return;
    titleAccumulator_ = 0.0;
    std::ostringstream title;
    title << baseTitle_ << " | " << std::fixed << std::setprecision(0) << fps_ << " FPS";
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(solarSystem_.bodies().size())) {
        title << " | Selected: " << solarSystem_.bodies()[static_cast<std::size_t>(selectedIndex_)].name;
    }
    title << " | F1 Help";
    glfwSetWindowTitle(window_, title.str().c_str());
}

void Application::toggleFullscreen() {
    if (window_ == nullptr) return;
    if (!fullscreen_) {
        glfwGetWindowPos(window_, &windowedX_, &windowedY_);
        glfwGetWindowSize(window_, &windowedWidth_, &windowedHeight_);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
        if (monitor && mode) {
            glfwSetWindowMonitor(window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            fullscreen_ = true;
        }
    } else {
        glfwSetWindowMonitor(window_, nullptr, windowedX_, windowedY_, windowedWidth_, windowedHeight_, 0);
        fullscreen_ = false;
    }
    glfwGetFramebufferSize(window_, &width_, &height_);
    width_ = std::max(width_, 1);
    height_ = std::max(height_, 1);
}

int Application::pickBody(double mouseX, double mouseY) const {
    const float x = static_cast<float>(2.0 * mouseX / static_cast<double>(std::max(width_, 1)) - 1.0);
    const float y = static_cast<float>(1.0 - 2.0 * mouseY / static_cast<double>(std::max(height_, 1)));
    const float aspect = static_cast<float>(std::max(width_, 1)) / static_cast<float>(std::max(height_, 1));
    const glm::mat4 projection = glm::perspective(glm::radians(camera_.zoomDegrees()), aspect, 0.01f, 950.0f);
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    const glm::vec3 rayDirection = safeNormalize(glm::vec3(glm::inverse(camera_.viewMatrix()) * rayEye));
    const glm::vec3 rayOrigin = camera_.position();

    int bestIndex = -1;
    float bestDistance = std::numeric_limits<float>::max();
    for (std::size_t i = 0; i < solarSystem_.bodies().size(); ++i) {
        const CelestialBody& current = solarSystem_.bodies()[i];
        const glm::vec3 oc = rayOrigin - current.worldPosition;
        const float radius = current.renderRadius * 1.25f;
        const float b = glm::dot(oc, rayDirection);
        const float c = glm::dot(oc, oc) - radius * radius;
        const float discriminant = b * b - c;
        if (discriminant < 0.0f) continue;
        const float root = std::sqrt(discriminant);
        float t = -b - root;
        if (t < 0.0f) t = -b + root;
        if (t >= 0.0f && t < bestDistance) {
            bestDistance = t;
            bestIndex = static_cast<int>(i);
        }
    }
    return bestIndex;
}

Application* Application::fromWindow(GLFWwindow* window) {
    return static_cast<Application*>(glfwGetWindowUserPointer(window));
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Application* application = fromWindow(window);
    if (!application) return;
    application->width_ = std::max(width, 1);
    application->height_ = std::max(height, 1);
}

void Application::cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition) {
    Application* application = fromWindow(window);
    if (!application) return;
    if (application->firstMouse_) {
        application->lastMouseX_ = xPosition;
        application->lastMouseY_ = yPosition;
        application->firstMouse_ = false;
    }
    const float xOffset = static_cast<float>(xPosition - application->lastMouseX_);
    const float yOffset = static_cast<float>(application->lastMouseY_ - yPosition);
    application->lastMouseX_ = xPosition;
    application->lastMouseY_ = yPosition;
    if (application->cursorCaptured_) application->camera_.processMouse(xOffset, yOffset);
}

void Application::scrollCallback(GLFWwindow* window, double, double yOffset) {
    Application* application = fromWindow(window);
    if (application) application->camera_.processScroll(static_cast<float>(yOffset));
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    Application* application = fromWindow(window);
    if (!application || action != GLFW_PRESS) return;
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        application->setCursorCaptured(!application->cursorCaptured_);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && !application->cursorCaptured_) {
        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);
        const int picked = application->pickBody(x, y);
        if (picked >= 0) application->selectBody(picked);
    }
}

} // namespace cosmosim
