#include "Spacecraft.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "MathUtils.h"
#include "SolarSystem.h"

namespace cosmosim {

void Spacecraft::launch(int originIndex, int targetIndex, const SolarSystem& system) {
    const auto& bodies = system.bodies();
    if (originIndex < 0 || targetIndex < 0 || originIndex == targetIndex ||
        originIndex >= static_cast<int>(bodies.size()) || targetIndex >= static_cast<int>(bodies.size())) {
        return;
    }

    telemetry_ = {};
    telemetry_.active = true;
    telemetry_.originIndex = originIndex;
    telemetry_.targetIndex = targetIndex;
    telemetry_.fuelPercent = 100.0f;
    launchPoint_ = bodies[static_cast<std::size_t>(originIndex)].worldPosition;
    const glm::vec3 outward = safeNormalize(launchPoint_, glm::vec3(1.0f, 0.0f, 0.0f));
    launchPoint_ += outward * bodies[static_cast<std::size_t>(originIndex)].renderRadius * 1.5f;
    lastPosition_ = launchPoint_;
    telemetry_.position = launchPoint_;
    computeControlPoints(bodies[static_cast<std::size_t>(targetIndex)].worldPosition);

    const float straightDistance = glm::length(bodies[static_cast<std::size_t>(targetIndex)].worldPosition - launchPoint_);
    durationSeconds_ = std::clamp(5.0f + straightDistance * 0.24f, 7.0f, 22.0f);
    elapsedSeconds_ = 0.0f;
    trail_.clear();
    trail_.push_back(launchPoint_);
}

void Spacecraft::abort() {
    telemetry_.active = false;
    telemetry_.completed = false;
    telemetry_.velocity = glm::vec3(0.0f);
    trail_.clear();
}

void Spacecraft::update(float deltaTime, const SolarSystem& system) {
    if (!telemetry_.active) return;
    const auto& bodies = system.bodies();
    if (telemetry_.targetIndex < 0 || telemetry_.targetIndex >= static_cast<int>(bodies.size())) {
        abort();
        return;
    }

    elapsedSeconds_ += deltaTime;
    telemetry_.progress = std::clamp(elapsedSeconds_ / durationSeconds_, 0.0f, 1.0f);
    const float eased = telemetry_.progress * telemetry_.progress * (3.0f - 2.0f * telemetry_.progress);
    const glm::vec3 destination = bodies[static_cast<std::size_t>(telemetry_.targetIndex)].worldPosition;
    computeControlPoints(destination);

    telemetry_.position = bezier3(launchPoint_, control1_, control2_, destination, eased);
    telemetry_.velocity = deltaTime > 0.00001f ? (telemetry_.position - lastPosition_) / deltaTime : glm::vec3(0.0f);
    telemetry_.speedUnitsPerSecond = glm::length(telemetry_.velocity);
    telemetry_.remainingDistance = glm::length(destination - telemetry_.position);
    telemetry_.fuelPercent = std::max(0.0f, 100.0f - telemetry_.progress * 86.0f);
    lastPosition_ = telemetry_.position;

    if (trail_.empty() || glm::dot(trail_.back() - telemetry_.position, trail_.back() - telemetry_.position) > 0.0025f) {
        trail_.push_back(telemetry_.position);
        while (trail_.size() > 700) trail_.pop_front();
    }

    const float targetRadius = bodies[static_cast<std::size_t>(telemetry_.targetIndex)].renderRadius;
    if (telemetry_.progress >= 1.0f || telemetry_.remainingDistance < targetRadius * 0.65f) {
        telemetry_.active = false;
        telemetry_.completed = true;
        telemetry_.progress = 1.0f;
        telemetry_.fuelPercent = std::max(telemetry_.fuelPercent, 12.0f);
        telemetry_.position = destination;
        telemetry_.velocity = glm::vec3(0.0f);
    }
}

glm::mat4 Spacecraft::modelMatrix() const {
    glm::vec3 direction = safeNormalize(telemetry_.velocity, glm::vec3(0.0f, 0.0f, 1.0f));
    const glm::vec3 upReference = std::abs(glm::dot(direction, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.95f
        ? glm::vec3(1.0f, 0.0f, 0.0f)
        : glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 right = safeNormalize(glm::cross(upReference, direction));
    const glm::vec3 up = safeNormalize(glm::cross(direction, right));
    glm::mat4 rotation(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(direction, 0.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), telemetry_.position) * rotation;
    return glm::scale(model, glm::vec3(0.34f));
}

std::vector<glm::vec3> Spacecraft::trajectoryPoints(const SolarSystem& system, unsigned int segments) const {
    std::vector<glm::vec3> points;
    if (telemetry_.targetIndex < 0 || telemetry_.targetIndex >= static_cast<int>(system.bodies().size())) return points;
    const glm::vec3 destination = system.bodies()[static_cast<std::size_t>(telemetry_.targetIndex)].worldPosition;
    points.reserve(segments + 1);
    for (unsigned int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        points.push_back(bezier3(launchPoint_, control1_, control2_, destination, t));
    }
    return points;
}

void Spacecraft::computeControlPoints(const glm::vec3& destination) {
    const glm::vec3 chord = destination - launchPoint_;
    const float distance = glm::length(chord);
    const glm::vec3 midpoint = (launchPoint_ + destination) * 0.5f;
    glm::vec3 radial = safeNormalize(midpoint, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 arcNormal = safeNormalize(glm::cross(safeNormalize(chord), glm::vec3(0.0f, 1.0f, 0.0f)), radial);
    if (glm::dot(arcNormal, radial) < 0.0f) arcNormal = -arcNormal;
    const glm::vec3 lift = radial * (3.0f + distance * 0.24f) + glm::vec3(0.0f, std::min(8.0f, distance * 0.16f), 0.0f);
    control1_ = launchPoint_ + chord * 0.30f + lift;
    control2_ = launchPoint_ + chord * 0.70f + lift * 0.82f;
}

} // namespace cosmosim
