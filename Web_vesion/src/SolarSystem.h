#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "SceneTypes.h"

namespace cosmosim {

class SolarSystem {
public:
    SolarSystem();

    void update(double deltaSeconds, SimulationSettings& settings, const glm::vec3& cameraPosition);
    void resetEpoch(SimulationSettings& settings);

    std::vector<CelestialBody>& bodies() { return bodies_; }
    const std::vector<CelestialBody>& bodies() const { return bodies_; }
    const std::vector<int>& selectableIndices() const { return selectableIndices_; }

    int indexByName(const std::string& name) const;
    int nextSelectable(int current, int direction) const;
    glm::vec3 orbitalPosition(const CelestialBody& body, double simulationDay, float distanceScale) const;
    float renderRadius(const CelestialBody& body, const SimulationSettings& settings) const;
    float maximumOrbitRadius(const SimulationSettings& settings) const;

private:
    std::vector<CelestialBody> bodies_;
    std::vector<int> selectableIndices_;

    void createBodies();
    void updateBodyRecursive(int index, double simulationDay, const SimulationSettings& settings, std::vector<bool>& visited);
    static glm::vec3 colorFromHex(unsigned int hex);
};

} // namespace cosmosim
