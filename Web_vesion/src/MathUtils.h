#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace cosmosim {

constexpr double kPi = 3.1415926535897932384626433832795;
constexpr double kTau = 2.0 * kPi;

inline double degToRad(double degrees) {
    return degrees * kPi / 180.0;
}

inline float degToRad(float degrees) {
    return degrees * glm::pi<float>() / 180.0f;
}

inline double wrapRadians(double radians) {
    radians = std::fmod(radians, kTau);
    if (radians < 0.0) {
        radians += kTau;
    }
    return radians;
}

template <typename T>
inline T clamp(T value, T minimum, T maximum) {
    return std::max(minimum, std::min(maximum, value));
}

inline float smoothstep(float edge0, float edge1, float x) {
    const float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline double solveEccentricAnomaly(double meanAnomaly, double eccentricity) {
    meanAnomaly = wrapRadians(meanAnomaly);
    double eccentricAnomaly = eccentricity < 0.8 ? meanAnomaly : kPi;
    for (int i = 0; i < 8; ++i) {
        const double f = eccentricAnomaly - eccentricity * std::sin(eccentricAnomaly) - meanAnomaly;
        const double derivative = 1.0 - eccentricity * std::cos(eccentricAnomaly);
        eccentricAnomaly -= f / derivative;
    }
    return eccentricAnomaly;
}

inline std::mt19937 makeRng(std::uint32_t seed = 0xC05A10u) {
    return std::mt19937(seed);
}

inline float randomRange(std::mt19937& rng, float minimum, float maximum) {
    return std::uniform_real_distribution<float>(minimum, maximum)(rng);
}

inline glm::vec3 safeNormalize(const glm::vec3& value, const glm::vec3& fallback = glm::vec3(0.0f, 1.0f, 0.0f)) {
    const float lengthSquared = glm::dot(value, value);
    if (lengthSquared < 1.0e-10f) {
        return fallback;
    }
    return value / std::sqrt(lengthSquared);
}

inline glm::vec3 bezier3(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
    const float u = 1.0f - t;
    return u * u * u * p0 + 3.0f * u * u * t * p1 + 3.0f * u * t * t * p2 + t * t * t * p3;
}

inline glm::vec3 bezier3Tangent(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
    const float u = 1.0f - t;
    return 3.0f * u * u * (p1 - p0) + 6.0f * u * t * (p2 - p1) + 3.0f * t * t * (p3 - p2);
}

} // namespace cosmosim
