#include "Geometry.h"

#include <cmath>
#include <cstdint>
#include <random>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "MathUtils.h"

namespace cosmosim::geometry {

Mesh makeUvSphere(unsigned int longitudeSegments, unsigned int latitudeSegments) {
    longitudeSegments = std::max(longitudeSegments, 8u);
    latitudeSegments = std::max(latitudeSegments, 4u);
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(static_cast<std::size_t>((longitudeSegments + 1) * (latitudeSegments + 1)));

    for (unsigned int y = 0; y <= latitudeSegments; ++y) {
        const float v = static_cast<float>(y) / static_cast<float>(latitudeSegments);
        const float phi = v * glm::pi<float>();
        for (unsigned int x = 0; x <= longitudeSegments; ++x) {
            const float u = static_cast<float>(x) / static_cast<float>(longitudeSegments);
            const float theta = u * glm::two_pi<float>();
            const glm::vec3 position(
                std::sin(phi) * std::cos(theta),
                std::cos(phi),
                std::sin(phi) * std::sin(theta));
            vertices.push_back({position, position, glm::vec2(u, v)});
        }
    }

    const unsigned int stride = longitudeSegments + 1;
    for (unsigned int y = 0; y < latitudeSegments; ++y) {
        for (unsigned int x = 0; x < longitudeSegments; ++x) {
            const std::uint32_t a = y * stride + x;
            const std::uint32_t b = a + stride;
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);
            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }
    return Mesh(vertices, indices);
}

Mesh makeLowPolyRock(unsigned int seed) {
    constexpr unsigned int longitudeSegments = 10;
    constexpr unsigned int latitudeSegments = 6;
    auto rng = makeRng(seed);
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    for (unsigned int y = 0; y <= latitudeSegments; ++y) {
        const float v = static_cast<float>(y) / static_cast<float>(latitudeSegments);
        const float phi = v * glm::pi<float>();
        for (unsigned int x = 0; x <= longitudeSegments; ++x) {
            const float u = static_cast<float>(x) / static_cast<float>(longitudeSegments);
            const float theta = u * glm::two_pi<float>();
            const float distortion = 0.78f + randomRange(rng, 0.0f, 0.42f);
            glm::vec3 position(
                std::sin(phi) * std::cos(theta),
                std::cos(phi),
                std::sin(phi) * std::sin(theta));
            position *= distortion;
            vertices.push_back({position, safeNormalize(position), glm::vec2(u, v)});
        }
    }

    const unsigned int stride = longitudeSegments + 1;
    for (unsigned int y = 0; y < latitudeSegments; ++y) {
        for (unsigned int x = 0; x < longitudeSegments; ++x) {
            const std::uint32_t a = y * stride + x;
            const std::uint32_t b = a + stride;
            indices.insert(indices.end(), {a, b, a + 1, a + 1, b, b + 1});
        }
    }
    return Mesh(vertices, indices);
}

Mesh makeRing(unsigned int segments) {
    segments = std::max(segments, 16u);
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(static_cast<std::size_t>((segments + 1) * 2));
    for (unsigned int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float angle = t * glm::two_pi<float>();
        const glm::vec3 radial(std::cos(angle), 0.0f, std::sin(angle));
        vertices.push_back({radial, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, t)});
        vertices.push_back({radial * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, t)});
    }
    for (unsigned int i = 0; i < segments; ++i) {
        const std::uint32_t a = i * 2;
        indices.insert(indices.end(), {a, a + 1, a + 2, a + 2, a + 1, a + 3});
    }
    return Mesh(vertices, indices);
}

Mesh makeSpacecraft() {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    auto addTriangle = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        const glm::vec3 normal = safeNormalize(glm::cross(b - a, c - a));
        const std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
        vertices.push_back({a, normal, {0.0f, 0.0f}});
        vertices.push_back({b, normal, {1.0f, 0.0f}});
        vertices.push_back({c, normal, {0.5f, 1.0f}});
        indices.insert(indices.end(), {base, base + 1, base + 2});
    };

    const glm::vec3 nose(0.0f, 0.0f, 1.8f);
    const glm::vec3 tail(0.0f, 0.0f, -1.2f);
    const glm::vec3 left(-0.55f, -0.25f, -0.65f);
    const glm::vec3 right(0.55f, -0.25f, -0.65f);
    const glm::vec3 top(0.0f, 0.48f, -0.55f);
    const glm::vec3 bottom(0.0f, -0.38f, -0.55f);
    addTriangle(nose, left, top);
    addTriangle(nose, top, right);
    addTriangle(nose, right, bottom);
    addTriangle(nose, bottom, left);
    addTriangle(tail, top, left);
    addTriangle(tail, right, top);
    addTriangle(tail, bottom, right);
    addTriangle(tail, left, bottom);

    const glm::vec3 wingL(-1.45f, -0.12f, -0.55f);
    const glm::vec3 wingR(1.45f, -0.12f, -0.55f);
    addTriangle(left, wingL, tail);
    addTriangle(tail, wingL, glm::vec3(-0.45f, -0.18f, 0.15f));
    addTriangle(right, tail, wingR);
    addTriangle(tail, glm::vec3(0.45f, -0.18f, 0.15f), wingR);
    return Mesh(vertices, indices);
}

Mesh makeOrbitLine(const OrbitalElements& orbit, float distanceScale, unsigned int segments) {
    std::vector<glm::vec3> points;
    points.reserve(segments);
    const double a = orbit.semiMajorAxis * static_cast<double>(distanceScale);
    const double b = a * std::sqrt(1.0 - orbit.eccentricity * orbit.eccentricity);
    const glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), degToRad(static_cast<float>(orbit.longitudeAscendingNodeDeg)), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), degToRad(static_cast<float>(orbit.inclinationDeg)), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), degToRad(static_cast<float>(orbit.argumentOfPeriapsisDeg)), glm::vec3(0.0f, 1.0f, 0.0f));
    for (unsigned int i = 0; i < segments; ++i) {
        const double angle = kTau * static_cast<double>(i) / static_cast<double>(segments);
        const glm::vec3 local(
            static_cast<float>(a * (std::cos(angle) - orbit.eccentricity)),
            0.0f,
            static_cast<float>(b * std::sin(angle)));
        points.push_back(glm::vec3(rotation * glm::vec4(local, 1.0f)));
    }
    Mesh mesh;
    mesh.uploadPositions(points, GL_LINE_LOOP);
    return mesh;
}

std::vector<glm::vec3> makeCirclePoints(float radius, unsigned int segments) {
    std::vector<glm::vec3> points;
    points.reserve(segments);
    for (unsigned int i = 0; i < segments; ++i) {
        const float angle = glm::two_pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        points.emplace_back(std::cos(angle) * radius, 0.0f, std::sin(angle) * radius);
    }
    return points;
}

} // namespace cosmosim::geometry
