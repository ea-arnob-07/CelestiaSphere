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
        if (milkyWayBias < 0.52f) {
            // Denser Milky Way band
            z = std::clamp(randomRange(rng, -0.18f, 0.18f) + std::sin(angle * 2.0f) * 0.12f, -1.0f, 1.0f);
        }
        const float xy = std::sqrt(std::max(0.0f, 1.0f - z * z));

        // Three shells for rich star density at all zoom levels:
        // Inner shell (100-300): visible when zoomed close to planets/moons
        // Mid shell (300-600): visible at solar system scale
        // Far shell (600-950): deep space backdrop
        const float shellPick = randomRange(rng, 0.0f, 1.0f);
        float radius;
        if (shellPick < 0.30f) {
            radius = randomRange(rng, 100.0f, 300.0f);   // inner - close zoom
        } else if (shellPick < 0.65f) {
            radius = randomRange(rng, 300.0f, 600.0f);   // mid
        } else {
            radius = randomRange(rng, 600.0f, 950.0f);   // far
        }

        // Realistic stellar luminosity: power-law (most faint, few bright)
        const float rawBrightness = randomRange(rng, 0.0f, 1.0f);
        const float brightness = std::pow(rawBrightness, 2.1f) * 0.78f + rawBrightness * 0.22f;

        // Realistic star sizes: mostly tiny dots, a few giants
        const float sizeRoll = randomRange(rng, 0.0f, 1.0f);
        float size;
        if (sizeRoll < 0.78f) {
            size = 0.7f + sizeRoll * 0.8f;
        } else if (sizeRoll < 0.94f) {
            size = 1.3f + (sizeRoll - 0.78f) * 7.0f;
        } else {
            size = 2.6f + std::pow(sizeRoll - 0.94f, 0.5f) * 9.0f;
        }
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
