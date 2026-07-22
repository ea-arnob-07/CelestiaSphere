#pragma once

#include <deque>
#include <vector>

#include <glm/glm.hpp>

#include "SceneTypes.h"

namespace cosmosim {

class SolarSystem;

class Spacecraft {
public:
    void launch(int originIndex, int targetIndex, const SolarSystem& system);
    void abort();
    void update(float deltaTime, const SolarSystem& system);

    const MissionTelemetry& telemetry() const { return telemetry_; }
    const std::deque<glm::vec3>& trail() const { return trail_; }
    glm::mat4 modelMatrix() const;
    std::vector<glm::vec3> trajectoryPoints(const SolarSystem& system, unsigned int segments = 160) const;

private:
    MissionTelemetry telemetry_;
    glm::vec3 launchPoint_ = glm::vec3(0.0f);
    glm::vec3 control1_ = glm::vec3(0.0f);
    glm::vec3 control2_ = glm::vec3(0.0f);
    glm::vec3 lastPosition_ = glm::vec3(0.0f);
    float durationSeconds_ = 12.0f;
    float elapsedSeconds_ = 0.0f;
    std::deque<glm::vec3> trail_;

    void computeControlPoints(const glm::vec3& destination);
};

} // namespace cosmosim
