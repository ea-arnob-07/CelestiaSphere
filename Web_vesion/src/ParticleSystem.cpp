#include "ParticleSystem.h"

#include <algorithm>
#include <cstddef>
#include <cmath>

#include <glm/gtc/constants.hpp>

#include "MathUtils.h"
#include "Shader.h"

namespace cosmosim {

ParticleSystem::~ParticleSystem() {
    reset();
}

void ParticleSystem::initialize(std::size_t maximumParticles) {
    reset();
    maximumParticles_ = maximumParticles;
    particles_.resize(maximumParticles_);
    gpuParticles_.reserve(maximumParticles_);
    rng_ = makeRng(0x51A7F11u);

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(maximumParticles_ * sizeof(GpuParticle)), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuParticle), reinterpret_cast<void*>(offsetof(GpuParticle, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GpuParticle), reinterpret_cast<void*>(offsetof(GpuParticle, color)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GpuParticle), reinterpret_cast<void*>(offsetof(GpuParticle, size)));
    glBindVertexArray(0);
}

void ParticleSystem::update(float deltaTime, const glm::vec3& sunPosition, float sunRadius, const MissionTelemetry& mission) {
    sunSpawnAccumulator_ += deltaTime * 150.0f;
    while (sunSpawnAccumulator_ >= 1.0f) {
        spawnSunParticle(sunPosition, sunRadius);
        sunSpawnAccumulator_ -= 1.0f;
    }
    if (mission.active) {
        engineSpawnAccumulator_ += deltaTime * 120.0f;
        while (engineSpawnAccumulator_ >= 1.0f) {
            spawnEngineParticle(mission);
            engineSpawnAccumulator_ -= 1.0f;
        }
    }

    for (Particle& particle : particles_) {
        if (particle.life <= 0.0f) continue;
        particle.life -= deltaTime;
        if (particle.life <= 0.0f) continue;
        particle.position += particle.velocity * deltaTime;
        particle.velocity *= std::pow(0.78f, deltaTime);
        particle.color.a = clamp(particle.life / particle.maxLife, 0.0f, 1.0f);
    }
}

void ParticleSystem::draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) const {
    gpuParticles_.clear();
    for (const Particle& particle : particles_) {
        if (particle.life > 0.0f) {
            gpuParticles_.push_back({particle.position, particle.color, particle.size});
        }
    }
    if (gpuParticles_.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(gpuParticles_.size() * sizeof(GpuParticle)), gpuParticles_.data());
    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);
    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(gpuParticles_.size()));
    glBindVertexArray(0);
}

void ParticleSystem::spawnSunParticle(const glm::vec3& sunPosition, float radius) {
    Particle* particle = findDeadParticle();
    if (!particle) return;
    const float z = randomRange(rng_, -1.0f, 1.0f);
    const float angle = randomRange(rng_, 0.0f, glm::two_pi<float>());
    const float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
    const glm::vec3 direction(r * std::cos(angle), z, r * std::sin(angle));
    particle->position = sunPosition + direction * radius * randomRange(rng_, 0.96f, 1.08f);
    particle->velocity = direction * randomRange(rng_, 0.35f, 1.8f);
    particle->color = glm::vec4(1.0f, randomRange(rng_, 0.35f, 0.78f), 0.05f, 1.0f);
    particle->size = randomRange(rng_, 2.0f, 8.0f);
    particle->life = randomRange(rng_, 0.5f, 1.8f);
    particle->maxLife = particle->life;
}

void ParticleSystem::spawnEngineParticle(const MissionTelemetry& mission) {
    Particle* particle = findDeadParticle();
    if (!particle) return;
    const glm::vec3 backward = -safeNormalize(mission.velocity, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 jitter(
        randomRange(rng_, -0.18f, 0.18f),
        randomRange(rng_, -0.18f, 0.18f),
        randomRange(rng_, -0.18f, 0.18f));
    particle->position = mission.position + backward * 0.35f + jitter;
    particle->velocity = backward * randomRange(rng_, 2.0f, 5.5f) + jitter * 2.0f;
    particle->color = glm::vec4(0.2f, randomRange(rng_, 0.55f, 0.95f), 1.0f, 1.0f);
    particle->size = randomRange(rng_, 3.0f, 7.0f);
    particle->life = randomRange(rng_, 0.25f, 0.85f);
    particle->maxLife = particle->life;
}

ParticleSystem::Particle* ParticleSystem::findDeadParticle() {
    for (Particle& particle : particles_) {
        if (particle.life <= 0.0f) return &particle;
    }
    return nullptr;
}

void ParticleSystem::reset() {
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = 0;
    particles_.clear();
    gpuParticles_.clear();
    maximumParticles_ = 0;
}

} // namespace cosmosim
