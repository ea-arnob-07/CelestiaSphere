#pragma once

#include <glm/glm.hpp>

namespace cosmosim {

enum class CameraMode {
    Free,
    Follow,
    TopDown,
    Cinematic
};

class Camera {
public:
    Camera();

    void update(float deltaTime, const glm::vec3& focusPoint, float focusRadius);
    void processKeyboard(const glm::vec3& direction, float deltaTime, bool boost);
    void processMouse(float xOffset, float yOffset);
    void processScroll(float yOffset);

    void setMode(CameraMode mode);
    CameraMode mode() const { return mode_; }

    void setPosition(const glm::vec3& position) { position_ = position; }
    void setYawPitch(float yaw, float pitch);
    void frameTarget(const glm::vec3& target, float radius);

    glm::mat4 viewMatrix() const;
    glm::vec3 position() const { return position_; }
    glm::vec3 front() const { return front_; }
    glm::vec3 right() const { return right_; }
    glm::vec3 up() const { return up_; }
    float zoomDegrees() const { return zoomDegrees_; }
    float followDistance() const { return followDistance_; }

private:
    CameraMode mode_ = CameraMode::Free;
    glm::vec3 position_ = glm::vec3(0.0f, 22.0f, 65.0f);
    glm::vec3 worldUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front_ = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right_ = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw_ = -90.0f;
    float pitch_ = -12.0f;
    float movementSpeed_ = 26.0f;
    float mouseSensitivity_ = 0.10f;
    float zoomDegrees_ = 48.0f;
    float followDistance_ = 12.0f;
    float cinematicAngle_ = 0.0f;
    glm::vec3 smoothedFocus_ = glm::vec3(0.0f);

    void updateVectors();
};

} // namespace cosmosim
