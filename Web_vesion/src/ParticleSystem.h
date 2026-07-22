#pragma once

#include <vector>
#include <random>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "SceneTypes.h"

namespace cosmosim {

class Shader;

class ParticleSystem {
public:
    ParticleSystem() = default;
    ~ParticleSystem();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    void initialize(std::size_t maximumParticles = 1800);
    void update(float deltaTime, const glm::vec3& sunPosition, float sunRadius, const MissionTelemetry& mission);
    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) const;

private:
    struct Particle {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec4 color = glm::vec4(1.0f);
        float size = 4.0f;
        float life = 0.0f;
        float maxLife = 1.0f;
    };

    struct GpuParticle {
        glm::vec3 position;
        glm::vec4 color;
        float size;
    };

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    std::size_t maximumParticles_ = 0;
    std::vector<Particle> particles_;
    mutable std::vector<GpuParticle> gpuParticles_;
    float sunSpawnAccumulator_ = 0.0f;
    float engineSpawnAccumulator_ = 0.0f;
    std::mt19937 rng_;

    void spawnSunParticle(const glm::vec3& sunPosition, float radius);
    void spawnEngineParticle(const MissionTelemetry& mission);
    Particle* findDeadParticle();
    void reset();
};

} // namespace cosmosim
