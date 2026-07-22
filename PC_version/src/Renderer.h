#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Mesh.h"
#include "Shader.h"

namespace cosmosim {

class Camera;
class ParticleSystem;
class SolarSystem;
class Spacecraft;
class Starfield;
class UiOverlay;
struct SimulationSettings;

class Renderer {
public:
    bool initialize(const SolarSystem& system, const SimulationSettings& settings);
    void render(
        const SolarSystem& system,
        const Camera& camera,
        const SimulationSettings& settings,
        int selectedIndex,
        const Spacecraft& spacecraft,
        ParticleSystem& particles,
        const Starfield& starfield,
        UiOverlay& ui,
        int width,
        int height,
        float elapsedTime,
        float fps,
        bool showHelp);

    void rebuildOrbits(const SolarSystem& system, const SimulationSettings& settings);

private:
    struct AsteroidState {
        float radius = 0.0f;
        float angle = 0.0f;
        float speed = 0.0f;
        float height = 0.0f;
        float scale = 0.0f;
        glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
        float spin = 0.0f;
    };

    Mesh sphere_;
    Mesh rock_;
    Mesh ring_;
    Mesh spacecraftMesh_;
    Mesh trajectoryMesh_;
    Mesh trailMesh_;
    std::vector<Mesh> orbitMeshes_;

    Shader planetShader_;
    Shader unlitShader_;
    Shader lineShader_;
    Shader asteroidShader_;
    Shader atmosphereShader_;
    Shader particleShader_;
    Shader starShader_;
    Shader uiShader_;

    std::vector<AsteroidState> asteroids_;
    std::vector<glm::mat4> asteroidMatrices_;
    float orbitDistanceScale_ = -1.0f;

    void createAsteroids();
    void updateAsteroids(float elapsedTime);
    void drawBodies(const SolarSystem& system, const Camera& camera, const SimulationSettings& settings, const glm::mat4& view, const glm::mat4& projection, int selectedIndex, float elapsedTime);
    void drawOrbits(const SolarSystem& system, const SimulationSettings& settings, const glm::mat4& view, const glm::mat4& projection, int selectedIndex);
    void drawAsteroids(const glm::mat4& view, const glm::mat4& projection, float elapsedTime);
    void drawMission(const SolarSystem& system, const Spacecraft& spacecraft, const glm::mat4& view, const glm::mat4& projection);
    void drawUi(const SolarSystem& system, const Camera& camera, const SimulationSettings& settings, int selectedIndex, const Spacecraft& spacecraft, UiOverlay& ui, int width, int height, float fps, bool showHelp);

    static int bodyKindId(int kind);
};

} // namespace cosmosim
