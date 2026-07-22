#include "Starfield.h"

#include <algorithm>
#include <cstddef>
#include <cmath>

#include <glm/gtc/constants.hpp>

#include "MathUtils.h"
#include "Shader.h"

namespace cosmosim {

Starfield::~Starfield() {
    reset();
}

void Starfield::initialize(std::size_t count) {
    reset();
    auto rng = makeRng(0x57A2F13u);
    std::vector<StarVertex> stars;
    stars.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        float z = randomRange(rng, -1.0f, 1.0f);
        const float angle = randomRange(rng, 0.0f, glm::two_pi<float>());
        const float milkyWayBias = randomRange(rng, 0.0f, 1.0f);
        if (milkyWayBias < 0.42f) {
            z = std::clamp(randomRange(rng, -0.22f, 0.22f) + std::sin(angle * 2.0f) * 0.08f, -1.0f, 1.0f);
        }
        const float xy = std::sqrt(std::max(0.0f, 1.0f - z * z));
        const float radius = randomRange(rng, 190.0f, 340.0f);
        const float brightness = std::pow(randomRange(rng, 0.18f, 1.0f), 1.55f);
        const float size = 0.8f + std::pow(randomRange(rng, 0.0f, 1.0f), 3.0f) * 5.6f;
        stars.push_back({
            glm::vec3(xy * std::cos(angle), z, xy * std::sin(angle)) * radius,
            brightness,
            size,
            randomRange(rng, 0.0f, glm::two_pi<float>())
        });
    }
    count_ = static_cast<GLsizei>(stars.size());
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(stars.size() * sizeof(StarVertex)), stars.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StarVertex), reinterpret_cast<void*>(offsetof(StarVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(StarVertex), reinterpret_cast<void*>(offsetof(StarVertex, brightness)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(StarVertex), reinterpret_cast<void*>(offsetof(StarVertex, size)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(StarVertex), reinterpret_cast<void*>(offsetof(StarVertex, phase)));
    glBindVertexArray(0);
}

void Starfield::draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float time) const {
    shader.use();
    glm::mat4 rotationOnly = glm::mat4(glm::mat3(view));
    shader.setMat4("uView", rotationOnly);
    shader.setMat4("uProjection", projection);
    shader.setFloat("uTime", time);
    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, count_);
    glBindVertexArray(0);
}

void Starfield::reset() {
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = 0;
    count_ = 0;
}

} // namespace cosmosim
