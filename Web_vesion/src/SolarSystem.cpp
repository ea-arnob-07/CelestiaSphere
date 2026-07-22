#include "SolarSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <glm/gtc/matrix_transform.hpp>

#include "MathUtils.h"

namespace cosmosim {

namespace {

CelestialBody body(
    std::string name,
    BodyKind kind,
    int parent,
    OrbitalElements orbit,
    double radiusKm,
    double rotationHours,
    double axialTilt,
    double massEarths,
    double gravity,
    double temperature,
    int moons,
    glm::vec3 baseColor,
    glm::vec3 accentColor,
    float atmosphere = 0.0f,
    float ringInner = 0.0f,
    float ringOuter = 0.0f) {
    CelestialBody result;
    result.name = std::move(name);
    result.kind = kind;
    result.parentIndex = parent;
    result.orbit = orbit;
    result.radiusKm = radiusKm;
    result.rotationPeriodHours = rotationHours;
    result.axialTiltDeg = axialTilt;
    result.massEarths = massEarths;
    result.gravityMs2 = gravity;
    result.temperatureC = temperature;
    result.knownMoons = moons;
    result.baseColor = baseColor;
    result.accentColor = accentColor;
    result.atmosphere = atmosphere;
    result.ringInner = ringInner;
    result.ringOuter = ringOuter;
    return result;
}

OrbitalElements orbit(double axis, double eccentricity, double inclination, double node, double periapsis, double periodDays, double anomaly) {
    return {axis, eccentricity, inclination, node, periapsis, periodDays, anomaly};
}

} // namespace

SolarSystem::SolarSystem() {
    createBodies();
}

void SolarSystem::update(double deltaSeconds, SimulationSettings& settings, const glm::vec3& cameraPosition) {
    if (!settings.paused) {
        settings.simulationDay += deltaSeconds * settings.daysPerSecond;
    }
    settings.distanceScale = settings.educationalScale ? 1.0f : 1.35f;
    settings.bodyScale = settings.educationalScale ? 1.0f : 0.72f;

    std::vector<bool> visited(bodies_.size(), false);
    for (std::size_t i = 0; i < bodies_.size(); ++i) {
        updateBodyRecursive(static_cast<int>(i), settings.simulationDay, settings, visited);
    }
    for (CelestialBody& current : bodies_) {
        current.distanceFromCamera = glm::length(current.worldPosition - cameraPosition);
    }
}

void SolarSystem::resetEpoch(SimulationSettings& settings) {
    settings.simulationDay = 0.0;
}

int SolarSystem::indexByName(const std::string& name) const {
    for (std::size_t i = 0; i < bodies_.size(); ++i) {
        if (bodies_[i].name == name) return static_cast<int>(i);
    }
    return -1;
}

int SolarSystem::nextSelectable(int current, int direction) const {
    if (selectableIndices_.empty()) return -1;
    auto found = std::find(selectableIndices_.begin(), selectableIndices_.end(), current);
    int position = found == selectableIndices_.end() ? 0 : static_cast<int>(std::distance(selectableIndices_.begin(), found));
    const int count = static_cast<int>(selectableIndices_.size());
    position = (position + direction) % count;
    if (position < 0) position += count;
    return selectableIndices_[static_cast<std::size_t>(position)];
}

glm::vec3 SolarSystem::orbitalPosition(const CelestialBody& current, double simulationDay, float distanceScale) const {
    if (current.parentIndex < 0 || current.orbit.semiMajorAxis <= 0.0) {
        return glm::vec3(0.0f);
    }
    const double period = std::max(std::abs(current.orbit.orbitalPeriodDays), 0.01);
    const double direction = current.orbit.orbitalPeriodDays < 0.0 ? -1.0 : 1.0;
    const double meanMotion = kTau / period;
    const double meanAnomaly = degToRad(current.orbit.meanAnomalyAtEpochDeg) + meanMotion * simulationDay * direction;
    const double eccentricAnomaly = solveEccentricAnomaly(meanAnomaly, current.orbit.eccentricity);
    const double a = current.orbit.semiMajorAxis * static_cast<double>(distanceScale);
    const double b = a * std::sqrt(1.0 - current.orbit.eccentricity * current.orbit.eccentricity);
    const glm::vec3 local(
        static_cast<float>(a * (std::cos(eccentricAnomaly) - current.orbit.eccentricity)),
        0.0f,
        static_cast<float>(b * std::sin(eccentricAnomaly)));

    glm::mat4 orientation(1.0f);
    orientation = glm::rotate(orientation, degToRad(static_cast<float>(current.orbit.longitudeAscendingNodeDeg)), glm::vec3(0.0f, 1.0f, 0.0f));
    orientation = glm::rotate(orientation, degToRad(static_cast<float>(current.orbit.inclinationDeg)), glm::vec3(1.0f, 0.0f, 0.0f));
    orientation = glm::rotate(orientation, degToRad(static_cast<float>(current.orbit.argumentOfPeriapsisDeg)), glm::vec3(0.0f, 1.0f, 0.0f));
    return glm::vec3(orientation * glm::vec4(local, 1.0f));
}

float SolarSystem::renderRadius(const CelestialBody& current, const SimulationSettings& settings) const {
    float radius = 1.0f;
    if (current.kind == BodyKind::Star) {
        radius = 4.2f;
    } else if (current.kind == BodyKind::BlackHole) {
        radius = 3.1f;
    } else {
        const float normalized = static_cast<float>(current.radiusKm / 6371.0);
        radius = 0.46f + std::pow(std::max(normalized, 0.02f), 0.46f) * 0.82f;
    }
    if (current.kind == BodyKind::Moon) radius *= 0.73f;
    if (current.kind == BodyKind::DwarfPlanet) radius *= 0.64f;
    return radius * settings.bodyScale;
}

float SolarSystem::maximumOrbitRadius(const SimulationSettings& settings) const {
    float maximum = 0.0f;
    for (const CelestialBody& current : bodies_) {
        if (current.parentIndex == 0) {
            maximum = std::max(maximum, static_cast<float>(current.orbit.semiMajorAxis * (1.0 + current.orbit.eccentricity)) * settings.distanceScale);
        }
    }
    return maximum;
}

void SolarSystem::createBodies() {
    bodies_.clear();
    bodies_.reserve(32);

    bodies_.push_back(body("Sun", BodyKind::Star, -1, {}, 696340.0, 609.12, 7.25, 332946.0, 274.0, 5505.0, 0,
        colorFromHex(0xFDB813), colorFromHex(0xFF5F00)));
    bodies_.push_back(body("Mercury", BodyKind::Rocky, 0, orbit(7.0, 0.2056, 7.00, 48.33, 29.12, 87.969, 174.8),
        2439.7, 1407.6, 0.034, 0.0553, 3.70, 167.0, 0, colorFromHex(0x9A8F84), colorFromHex(0xD0C2B4)));
    bodies_.push_back(body("Venus", BodyKind::Rocky, 0, orbit(10.0, 0.0068, 3.39, 76.68, 54.88, 224.701, 50.1),
        6051.8, -5832.5, 177.36, 0.815, 8.87, 464.0, 0, colorFromHex(0xC98B3C), colorFromHex(0xF2D08C), 0.34f));
    bodies_.push_back(body("Earth", BodyKind::EarthLike, 0, orbit(13.5, 0.0167, 0.00, -11.26, 114.21, 365.256, 357.5),
        6371.0, 23.934, 23.44, 1.0, 9.81, 15.0, 1, colorFromHex(0x2C7BE5), colorFromHex(0x3BAA5C), 0.30f));
    bodies_.push_back(body("Moon", BodyKind::Moon, 3, orbit(2.25, 0.0549, 5.14, 125.08, 318.15, 27.322, 115.4),
        1737.4, 655.7, 6.68, 0.0123, 1.62, -20.0, 0, colorFromHex(0xA9A9A9), colorFromHex(0xE0E0D8)));
    bodies_.push_back(body("Mars", BodyKind::Rocky, 0, orbit(17.7, 0.0934, 1.85, 49.56, 286.50, 686.980, 19.4),
        3389.5, 24.623, 25.19, 0.107, 3.71, -63.0, 2, colorFromHex(0xA84D2A), colorFromHex(0xD88650), 0.10f));
    bodies_.push_back(body("Phobos", BodyKind::Moon, 5, orbit(1.45, 0.0151, 1.08, 0.0, 0.0, 0.319, 30.0),
        11.3, 7.66, 0.0, 1.78e-9, 0.0057, -40.0, 0, colorFromHex(0x796B5E), colorFromHex(0xA89B8C)));
    bodies_.push_back(body("Deimos", BodyKind::Moon, 5, orbit(1.95, 0.0002, 1.79, 0.0, 0.0, 1.263, 210.0),
        6.2, 30.3, 0.0, 2.4e-10, 0.0030, -40.0, 0, colorFromHex(0x8A7B6E), colorFromHex(0xB8AA99)));
    bodies_.push_back(body("Jupiter", BodyKind::GasGiant, 0, orbit(25.3, 0.0489, 1.30, 100.46, 273.87, 4332.59, 20.0),
        69911.0, 9.925, 3.13, 317.8, 24.79, -110.0, 95, colorFromHex(0xC69C72), colorFromHex(0xE7D0B1), 0.08f));
    bodies_.push_back(body("Io", BodyKind::Moon, 8, orbit(2.10, 0.0041, 0.05, 43.98, 84.13, 1.769, 10.0),
        1821.6, 42.5, 0.0, 0.0150, 1.796, -130.0, 0, colorFromHex(0xD9B650), colorFromHex(0xF2E3A5), 0.02f));
    bodies_.push_back(body("Europa", BodyKind::Moon, 8, orbit(2.70, 0.0090, 0.47, 219.11, 88.97, 3.551, 120.0),
        1560.8, 85.2, 0.1, 0.0080, 1.315, -160.0, 0, colorFromHex(0xB8A68F), colorFromHex(0xE7D7C3), 0.02f));
    bodies_.push_back(body("Ganymede", BodyKind::Moon, 8, orbit(3.35, 0.0013, 0.21, 63.55, 192.42, 7.154, 220.0),
        2634.1, 171.7, 0.3, 0.0250, 1.428, -163.0, 0, colorFromHex(0x7F776D), colorFromHex(0xB8AEA2), 0.01f));
    bodies_.push_back(body("Callisto", BodyKind::Moon, 8, orbit(4.15, 0.0074, 0.19, 298.85, 52.64, 16.689, 300.0),
        2410.3, 400.5, 0.0, 0.0180, 1.235, -139.0, 0, colorFromHex(0x6F6257), colorFromHex(0xA49487)));
    bodies_.push_back(body("Saturn", BodyKind::GasGiant, 0, orbit(33.0, 0.0565, 2.49, 113.67, 339.39, 10759.2, 317.0),
        58232.0, 10.656, 26.73, 95.16, 10.44, -140.0, 146, colorFromHex(0xD8C38F), colorFromHex(0xF0E3BD), 0.06f, 1.35f, 2.35f));
    bodies_.push_back(body("Titan", BodyKind::Moon, 13, orbit(2.55, 0.0288, 0.35, 168.65, 186.59, 15.945, 35.0),
        2574.7, 382.7, 0.3, 0.0225, 1.352, -179.0, 0, colorFromHex(0xC79A5C), colorFromHex(0xF0C980), 0.12f));
    bodies_.push_back(body("Enceladus", BodyKind::Moon, 13, orbit(1.95, 0.0047, 0.01, 0.0, 0.0, 1.370, 70.0),
        252.1, 32.9, 0.0, 0.000018, 0.113, -201.0, 0, colorFromHex(0xD7E4EE), colorFromHex(0xF5FAFF), 0.01f));
    bodies_.push_back(body("Uranus", BodyKind::IceGiant, 0, orbit(40.2, 0.0457, 0.77, 74.01, 96.99, 30688.5, 142.2),
        25362.0, -17.24, 97.77, 14.54, 8.69, -195.0, 28, colorFromHex(0x73C7D2), colorFromHex(0xB7EEF2), 0.12f, 1.55f, 1.85f));
    bodies_.push_back(body("Titania", BodyKind::Moon, 16, orbit(2.25, 0.0011, 0.34, 0.0, 0.0, 8.706, 20.0),
        788.9, 209.0, 0.0, 0.00059, 0.379, -203.0, 0, colorFromHex(0x9A958D), colorFromHex(0xC9C5BD)));
    bodies_.push_back(body("Oberon", BodyKind::Moon, 16, orbit(2.90, 0.0014, 0.07, 0.0, 0.0, 13.463, 200.0),
        761.4, 323.0, 0.0, 0.00050, 0.346, -203.0, 0, colorFromHex(0x7D766E), colorFromHex(0xB3ACA3)));
    bodies_.push_back(body("Neptune", BodyKind::IceGiant, 0, orbit(47.0, 0.0113, 1.77, 131.78, 273.19, 60182.0, 256.2),
        24622.0, 16.11, 28.32, 17.15, 11.15, -200.0, 16, colorFromHex(0x3154C7), colorFromHex(0x68A4FF), 0.12f));
    bodies_.push_back(body("Triton", BodyKind::Moon, 19, orbit(2.40, 0.0000, 156.9, 0.0, 0.0, -5.877, 140.0),
        1353.4, -141.0, 0.0, 0.0036, 0.779, -235.0, 0, colorFromHex(0xC7B8A8), colorFromHex(0xF0E6DB), 0.02f));
    bodies_.push_back(body("Pluto", BodyKind::DwarfPlanet, 0, orbit(54.0, 0.2488, 17.16, 110.30, 113.76, 90560.0, 14.5),
        1188.3, -153.3, 122.53, 0.00218, 0.62, -225.0, 5, colorFromHex(0xB09A86), colorFromHex(0xE1CDBA)));
    bodies_.push_back(body("Charon", BodyKind::Moon, 21, orbit(1.75, 0.0002, 0.08, 0.0, 0.0, 6.387, 200.0),
        606.0, 153.3, 0.0, 0.000254, 0.288, -220.0, 0, colorFromHex(0x92877A), colorFromHex(0xD7CABB)));
    bodies_.push_back(body("Black Hole", BodyKind::BlackHole, 0, orbit(92.0, 0.18, 24.0, 36.0, 15.0, 280000.0, 105.0),
        18000.0, 100.0, 0.0, 150000.0, 1000.0, 0.0, 0, colorFromHex(0x090511), colorFromHex(0xF68D3E), 0.0f, 1.7f, 3.0f));

    selectableIndices_ = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
    };
}

void SolarSystem::updateBodyRecursive(int index, double simulationDay, const SimulationSettings& settings, std::vector<bool>& visited) {
    if (index < 0 || index >= static_cast<int>(bodies_.size())) return;
    if (visited[static_cast<std::size_t>(index)]) return;

    CelestialBody& current = bodies_[static_cast<std::size_t>(index)];
    glm::vec3 parentPosition(0.0f);
    if (current.parentIndex >= 0) {
        updateBodyRecursive(current.parentIndex, simulationDay, settings, visited);
        parentPosition = bodies_[static_cast<std::size_t>(current.parentIndex)].worldPosition;
    }

    current.worldPosition = parentPosition + orbitalPosition(current, simulationDay, settings.distanceScale);
    current.renderRadius = renderRadius(current, settings);

    double rotationAngle = 0.0;
    if (std::abs(current.rotationPeriodHours) > 0.001) {
        rotationAngle = kTau * ((simulationDay * 24.0) / current.rotationPeriodHours);
    }
    glm::mat4 model(1.0f);
    model = glm::translate(model, current.worldPosition);
    model = glm::rotate(model, degToRad(static_cast<float>(current.axialTiltDeg)), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, static_cast<float>(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(current.renderRadius));
    current.modelMatrix = model;
    visited[static_cast<std::size_t>(index)] = true;
}

glm::vec3 SolarSystem::colorFromHex(unsigned int hex) {
    return glm::vec3(
        static_cast<float>((hex >> 16u) & 0xFFu) / 255.0f,
        static_cast<float>((hex >> 8u) & 0xFFu) / 255.0f,
        static_cast<float>(hex & 0xFFu) / 255.0f);
}

} // namespace cosmosim
