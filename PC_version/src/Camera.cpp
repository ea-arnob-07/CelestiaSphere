#include "Camera.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "MathUtils.h"

namespace cosmosim {

Camera::Camera() {
    updateVectors();
}

void Camera::update(float deltaTime, const glm::vec3& focusPoint, float focusRadius) {
    const float blend = 1.0f - std::exp(-6.0f * deltaTime);
    smoothedFocus_ = glm::mix(smoothedFocus_, focusPoint, blend);

    if (mode_ == CameraMode::Follow) {
        followDistance_ = std::max(followDistance_, focusRadius * 1.15f + 0.15f);
        const glm::vec3 desired = smoothedFocus_ - front_ * followDistance_ + worldUp_ * focusRadius * 0.35f;
        position_ = glm::mix(position_, desired, blend);
    } else if (mode_ == CameraMode::TopDown) {
        const float height = std::max(14.0f, followDistance_ * 3.8f);
        position_ = glm::mix(position_, smoothedFocus_ + glm::vec3(0.0f, height, 0.01f), blend);
        front_ = safeNormalize(smoothedFocus_ - position_);
        right_ = safeNormalize(glm::cross(front_, glm::vec3(0.0f, 0.0f, -1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
        up_ = safeNormalize(glm::cross(right_, front_));
    } else if (mode_ == CameraMode::Cinematic) {
        cinematicAngle_ += deltaTime * 0.16f;
        const float orbitDistance = std::max(8.0f, focusRadius * 3.0f + 2.5f);
        const glm::vec3 desired = smoothedFocus_ + glm::vec3(
            std::cos(cinematicAngle_) * orbitDistance,
            orbitDistance * 0.32f + std::sin(cinematicAngle_ * 0.7f) * 3.0f,
            std::sin(cinematicAngle_) * orbitDistance);
        position_ = glm::mix(position_, desired, blend * 0.6f);
        front_ = safeNormalize(smoothedFocus_ - position_);
        right_ = safeNormalize(glm::cross(front_, worldUp_));
        up_ = safeNormalize(glm::cross(right_, front_));
    }
}

void Camera::processKeyboard(const glm::vec3& direction, float deltaTime, bool boost) {
    if (mode_ != CameraMode::Free) {
        return;
    }
    const float speed = movementSpeed_ * (boost ? 3.5f : 1.0f) * deltaTime;
    position_ += front_ * direction.z * speed;
    position_ += right_ * direction.x * speed;
    position_ += worldUp_ * direction.y * speed;
}

void Camera::processMouse(float xOffset, float yOffset) {
    if (mode_ == CameraMode::TopDown || mode_ == CameraMode::Cinematic) {
        return;
    }
    yaw_ += xOffset * mouseSensitivity_;
    pitch_ += yOffset * mouseSensitivity_;
    pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    updateVectors();
}

void Camera::processScroll(float yOffset) {
    if (mode_ == CameraMode::Follow || mode_ == CameraMode::TopDown || mode_ == CameraMode::Cinematic) {
        followDistance_ *= std::pow(0.88f, yOffset);
        followDistance_ = std::clamp(followDistance_, 0.35f, 450.0f);
    } else {
        zoomDegrees_ -= yOffset * 1.5f;
        zoomDegrees_ = std::clamp(zoomDegrees_, 8.0f, 90.0f);
    }
}

void Camera::setMode(CameraMode mode) {
    mode_ = mode;
}

void Camera::setYawPitch(float yaw, float pitch) {
    yaw_ = yaw;
    pitch_ = std::clamp(pitch, -89.0f, 89.0f);
    updateVectors();
}

void Camera::frameTarget(const glm::vec3& target, float radius) {
    const float distance = std::max(1.6f, radius * 2.2f);
    smoothedFocus_ = target;
    position_ = target + glm::vec3(distance * 0.45f, distance * 0.22f, distance);
    front_ = safeNormalize(target - position_);
    yaw_ = glm::degrees(std::atan2(front_.z, front_.x)) - 90.0f;
    pitch_ = glm::degrees(std::asin(front_.y));
    followDistance_ = distance;
    updateVectors();
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::updateVectors() {
    glm::vec3 direction;
    direction.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    direction.y = std::sin(glm::radians(pitch_));
    direction.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front_ = safeNormalize(direction);
    right_ = safeNormalize(glm::cross(front_, worldUp_));
    up_ = safeNormalize(glm::cross(right_, front_));
}

} // namespace cosmosim
