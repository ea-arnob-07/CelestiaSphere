#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace cosmosim {

enum class BodyKind {
    Star,
    Rocky,
    EarthLike,
    GasGiant,
    IceGiant,
    Moon,
    DwarfPlanet,
    BlackHole
};

struct OrbitalElements {
    double semiMajorAxis = 0.0;
    double eccentricity = 0.0;
    double inclinationDeg = 0.0;
    double longitudeAscendingNodeDeg = 0.0;
    double argumentOfPeriapsisDeg = 0.0;
    double orbitalPeriodDays = 1.0;
    double meanAnomalyAtEpochDeg = 0.0;
};

struct CelestialBody {
    std::string name;
    BodyKind kind = BodyKind::Rocky;
    int parentIndex = -1;
    OrbitalElements orbit;
    double radiusKm = 1.0;
    double rotationPeriodHours = 24.0;
    double axialTiltDeg = 0.0;
    double massEarths = 0.0;
    double gravityMs2 = 0.0;
    double temperatureC = 0.0;
    int knownMoons = 0;
    glm::vec3 baseColor = glm::vec3(1.0f);
    glm::vec3 accentColor = glm::vec3(0.5f);
    float atmosphere = 0.0f;
    float ringInner = 0.0f;
    float ringOuter = 0.0f;
    bool visible = true;

    glm::vec3 worldPosition = glm::vec3(0.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    float renderRadius = 1.0f;
    float distanceFromCamera = 0.0f;
};

struct SimulationSettings {
    bool paused = false;
    bool showOrbits = true;
    bool showLabels = true;
    bool showAsteroids = true;
    bool showAtmospheres = true;
    bool educationalScale = true;
    bool cinematicTrails = true;
    double daysPerSecond = 3.0;
    double simulationDay = 0.0;
    float distanceScale = 1.0f;
    float bodyScale = 1.0f;
};

struct MissionTelemetry {
    bool active = false;
    bool completed = false;
    int originIndex = -1;
    int targetIndex = -1;
    float progress = 0.0f;
    float speedUnitsPerSecond = 0.0f;
    float remainingDistance = 0.0f;
    float fuelPercent = 100.0f;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
};

} // namespace cosmosim
