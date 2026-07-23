#include "Renderer.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Geometry.h"
#include "MathUtils.h"
#include "ParticleSystem.h"
#include "SceneTypes.h"
#include "SolarSystem.h"
#include "Spacecraft.h"
#include "Starfield.h"
#include "UiOverlay.h"

#ifndef COSMOSIM_SHADER_DIR
#define COSMOSIM_SHADER_DIR "shaders"
#endif

namespace cosmosim {

namespace {
std::string shaderPath(const char* filename) {
    return std::string(COSMOSIM_SHADER_DIR) + "/" + filename;
}

std::string fixed(double value, int precision = 1) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

std::string cameraModeName(CameraMode mode) {
    switch (mode) {
        case CameraMode::Free: return "FREE";
        case CameraMode::Follow: return "FOLLOW";
        case CameraMode::TopDown: return "TOP";
        case CameraMode::Cinematic: return "CINEMA";
    }
    return "UNKNOWN";
}

std::string bodyKindName(BodyKind kind) {
    switch (kind) {
        case BodyKind::Star: return "STAR";
        case BodyKind::Rocky: return "ROCKY PLANET";
        case BodyKind::EarthLike: return "EARTH-LIKE PLANET";
        case BodyKind::GasGiant: return "GAS GIANT";
        case BodyKind::IceGiant: return "ICE GIANT";
        case BodyKind::Moon: return "NATURAL SATELLITE";
        case BodyKind::DwarfPlanet: return "DWARF PLANET";
        case BodyKind::BlackHole: return "BLACK HOLE";
    }
    return "UNKNOWN";
}
}

bool Renderer::initialize(const SolarSystem& system, const SimulationSettings& settings) {
    sphere_ = geometry::makeUvSphere(72, 40);
    rock_ = geometry::makeLowPolyRock(77);
    ring_ = geometry::makeRing(256);
    spacecraftMesh_ = geometry::makeSpacecraft();

    const bool shadersOk =
        planetShader_.load(shaderPath("planet.vert"), shaderPath("planet.frag")) &&
        unlitShader_.load(shaderPath("unlit.vert"), shaderPath("unlit.frag")) &&
        lineShader_.load(shaderPath("line.vert"), shaderPath("line.frag")) &&
        asteroidShader_.load(shaderPath("asteroid.vert"), shaderPath("asteroid.frag")) &&
        atmosphereShader_.load(shaderPath("atmosphere.vert"), shaderPath("atmosphere.frag")) &&
        particleShader_.load(shaderPath("particle.vert"), shaderPath("particle.frag")) &&
        starShader_.load(shaderPath("star.vert"), shaderPath("star.frag")) &&
        uiShader_.load(shaderPath("ui.vert"), shaderPath("ui.frag"));
    if (!shadersOk) return false;

    createAsteroids();
    rock_.setInstanceMatrices(asteroidMatrices_, GL_DYNAMIC_DRAW);
    rebuildOrbits(system, settings);
    return true;
}

void Renderer::render(
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
    bool showHelp) {
    if (std::abs(settings.distanceScale - orbitDistanceScale_) > 0.001f) {
        rebuildOrbits(system, settings);
    }

    glViewport(0, 0, width, height);
    glClearColor(0.0007f, 0.0012f, 0.0055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(std::max(width, 1)) / static_cast<float>(std::max(height, 1));
    const glm::mat4 projection = glm::perspective(glm::radians(camera.zoomDegrees()), aspect, 0.01f, 950.0f);
    const glm::mat4 view = camera.viewMatrix();

    glDepthMask(GL_FALSE);
    starfield.draw(starShader_, view, projection, elapsedTime);
    glDepthMask(GL_TRUE);

    if (settings.showOrbits) drawOrbits(system, settings, view, projection, selectedIndex);
    if (settings.showAsteroids) drawAsteroids(view, projection, elapsedTime);
    drawBodies(system, camera, settings, view, projection, selectedIndex, elapsedTime);
    drawMission(system, spacecraft, view, projection);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    particles.draw(particleShader_, view, projection);
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawUi(system, camera, settings, selectedIndex, spacecraft, ui, width, height, fps, showHelp);
}

void Renderer::rebuildOrbits(const SolarSystem& system, const SimulationSettings& settings) {
    orbitMeshes_.clear();
    orbitMeshes_.reserve(system.bodies().size());
    for (const CelestialBody& current : system.bodies()) {
        if (current.parentIndex >= 0 && current.orbit.semiMajorAxis > 0.0) {
            orbitMeshes_.push_back(geometry::makeOrbitLine(current.orbit, settings.distanceScale, 300));
        } else {
            orbitMeshes_.emplace_back();
        }
    }
    orbitDistanceScale_ = settings.distanceScale;
}

void Renderer::createAsteroids() {
    auto rng = makeRng(0xA57E201u);
    constexpr std::size_t asteroidCount = 1500;
    asteroids_.reserve(asteroidCount);
    asteroidMatrices_.resize(asteroidCount, glm::mat4(1.0f));
    for (std::size_t i = 0; i < asteroidCount; ++i) {
        AsteroidState asteroid;
        asteroid.radius = randomRange(rng, 20.5f, 23.3f);
        if (randomRange(rng, 0.0f, 1.0f) < 0.14f) asteroid.radius += randomRange(rng, -1.3f, 1.3f);
        asteroid.angle = randomRange(rng, 0.0f, glm::two_pi<float>());
        asteroid.speed = randomRange(rng, 0.006f, 0.017f) * (randomRange(rng, 0.0f, 1.0f) < 0.03f ? -1.0f : 1.0f);
        asteroid.height = randomRange(rng, -0.72f, 0.72f);
        asteroid.scale = randomRange(rng, 0.035f, 0.16f);
        asteroid.axis = safeNormalize(glm::vec3(
            randomRange(rng, -1.0f, 1.0f),
            randomRange(rng, -1.0f, 1.0f),
            randomRange(rng, -1.0f, 1.0f)));
        asteroid.spin = randomRange(rng, -2.0f, 2.0f);
        asteroids_.push_back(asteroid);
    }
    updateAsteroids(0.0f);
}

void Renderer::updateAsteroids(float elapsedTime) {
    for (std::size_t i = 0; i < asteroids_.size(); ++i) {
        const AsteroidState& asteroid = asteroids_[i];
        const float angle = asteroid.angle + elapsedTime * asteroid.speed;
        const float wobble = std::sin(angle * 3.1f + asteroid.radius) * 0.15f;
        glm::mat4 model(1.0f);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(asteroid.radius, asteroid.height + wobble, 0.0f));
        model = glm::rotate(model, elapsedTime * asteroid.spin + asteroid.angle, asteroid.axis);
        model = glm::scale(model, glm::vec3(asteroid.scale));
        asteroidMatrices_[i] = model;
    }
    rock_.updateInstanceMatrices(asteroidMatrices_);
}

void Renderer::drawBodies(const SolarSystem& system, const Camera& camera, const SimulationSettings& settings, const glm::mat4& view, const glm::mat4& projection, int selectedIndex, float elapsedTime) {
    planetShader_.use();
    planetShader_.setMat4("uView", view);
    planetShader_.setMat4("uProjection", projection);
    planetShader_.setVec3("uCameraPosition", camera.position());
    planetShader_.setVec3("uSunPosition", system.bodies().front().worldPosition);
    planetShader_.setFloat("uTime", elapsedTime);

    for (std::size_t i = 0; i < system.bodies().size(); ++i) {
        const CelestialBody& current = system.bodies()[i];
        if (!current.visible) continue;
        planetShader_.use();
        planetShader_.setMat4("uModel", current.modelMatrix);
        planetShader_.setVec3("uBaseColor", current.baseColor);
        planetShader_.setVec3("uAccentColor", current.accentColor);
        planetShader_.setInt("uBodyKind", bodyKindId(static_cast<int>(current.kind)));
        planetShader_.setBool("uIsSun", current.kind == BodyKind::Star);
        planetShader_.setBool("uSelected", static_cast<int>(i) == selectedIndex);
        sphere_.draw();

        if (current.ringOuter > 0.0f) {
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
            unlitShader_.use();
            unlitShader_.setMat4("uView", view);
            unlitShader_.setMat4("uProjection", projection);
            glm::mat4 ringModel(1.0f);
            ringModel = glm::translate(ringModel, current.worldPosition);
            ringModel = glm::rotate(ringModel, degToRad(static_cast<float>(current.axialTiltDeg)), glm::vec3(0.0f, 0.0f, 1.0f));
            ringModel = glm::scale(ringModel, glm::vec3(current.renderRadius * current.ringInner, 1.0f, current.renderRadius * current.ringInner));
            unlitShader_.setMat4("uModel", ringModel);
            const glm::vec4 ringColor = current.kind == BodyKind::BlackHole
                ? glm::vec4(0.98f, 0.52f, 0.18f, 0.72f)
                : glm::vec4(glm::mix(current.baseColor, glm::vec3(0.9f), 0.45f), 0.48f);
            unlitShader_.setVec4("uColor", ringColor);
            unlitShader_.setBool("uRadialFade", true);
            ring_.draw();
            glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);
        }
    }

    if (settings.showAtmospheres) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        atmosphereShader_.use();
        atmosphereShader_.setMat4("uView", view);
        atmosphereShader_.setMat4("uProjection", projection);
        atmosphereShader_.setVec3("uCameraPosition", camera.position());
        for (const CelestialBody& current : system.bodies()) {
            if (current.atmosphere <= 0.0f) continue;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), current.worldPosition);
            model = glm::scale(model, glm::vec3(current.renderRadius * (1.04f + current.atmosphere * 0.12f)));
            atmosphereShader_.setMat4("uModel", model);
            atmosphereShader_.setVec3("uAtmosphereColor", glm::mix(current.baseColor, glm::vec3(0.45f, 0.75f, 1.0f), 0.6f));
            atmosphereShader_.setFloat("uStrength", current.atmosphere);
            sphere_.draw();
        }
        glDepthMask(GL_TRUE);
    }
}

void Renderer::drawOrbits(const SolarSystem& system, const SimulationSettings&, const glm::mat4& view, const glm::mat4& projection, int selectedIndex) {
    lineShader_.use();
    lineShader_.setMat4("uView", view);
    lineShader_.setMat4("uProjection", projection);
    lineShader_.setBool("uDashed", false);
    lineShader_.setFloat("uTime", 0.0f);
    glLineWidth(1.0f);

    const auto& bodies = system.bodies();
    for (std::size_t i = 0; i < bodies.size() && i < orbitMeshes_.size(); ++i) {
        const CelestialBody& current = bodies[i];
        if (current.parentIndex < 0 || !orbitMeshes_[i].valid()) continue;
        glm::mat4 model(1.0f);
        if (current.parentIndex > 0) {
            model = glm::translate(model, bodies[static_cast<std::size_t>(current.parentIndex)].worldPosition);
        }
        lineShader_.setMat4("uModel", model);
        const bool selected = static_cast<int>(i) == selectedIndex;
        lineShader_.setVec4("uColor", selected ? glm::vec4(0.35f, 0.85f, 1.0f, 0.90f) : glm::vec4(0.28f, 0.38f, 0.52f, 0.34f));
        orbitMeshes_[i].draw();
    }
}

void Renderer::drawAsteroids(const glm::mat4& view, const glm::mat4& projection, float elapsedTime) {
    updateAsteroids(elapsedTime);
    asteroidShader_.use();
    asteroidShader_.setMat4("uView", view);
    asteroidShader_.setMat4("uProjection", projection);
    asteroidShader_.setVec3("uSunPosition", glm::vec3(0.0f));
    asteroidShader_.setVec3("uColor", glm::vec3(0.34f, 0.30f, 0.27f));
    rock_.drawInstanced(static_cast<GLsizei>(asteroidMatrices_.size()));
}

void Renderer::drawMission(const SolarSystem& system, const Spacecraft& spacecraft, const glm::mat4& view, const glm::mat4& projection) {
    const MissionTelemetry& mission = spacecraft.telemetry();
    if (mission.targetIndex < 0) return;

    const std::vector<glm::vec3> trajectory = spacecraft.trajectoryPoints(system);
    if (!trajectory.empty()) {
        trajectoryMesh_.uploadPositions(trajectory, GL_LINE_STRIP);
        lineShader_.use();
        lineShader_.setMat4("uModel", glm::mat4(1.0f));
        lineShader_.setMat4("uView", view);
        lineShader_.setMat4("uProjection", projection);
        lineShader_.setVec4("uColor", glm::vec4(0.2f, 0.72f, 1.0f, 0.50f));
        lineShader_.setBool("uDashed", true);
        lineShader_.setFloat("uTime", mission.progress * 20.0f);
        trajectoryMesh_.draw();
    }

    if (!spacecraft.trail().empty()) {
        std::vector<glm::vec3> trail(spacecraft.trail().begin(), spacecraft.trail().end());
        trailMesh_.uploadPositions(trail, GL_LINE_STRIP);
        lineShader_.use();
        lineShader_.setMat4("uModel", glm::mat4(1.0f));
        lineShader_.setVec4("uColor", glm::vec4(0.35f, 0.9f, 1.0f, 0.8f));
        lineShader_.setBool("uDashed", false);
        trailMesh_.draw();
    }

    if (mission.active) {
        planetShader_.use();
        planetShader_.setMat4("uView", view);
        planetShader_.setMat4("uProjection", projection);
        planetShader_.setMat4("uModel", spacecraft.modelMatrix());
        planetShader_.setVec3("uCameraPosition", glm::vec3(0.0f));
        planetShader_.setVec3("uSunPosition", glm::vec3(0.0f));
        planetShader_.setVec3("uBaseColor", glm::vec3(0.72f, 0.78f, 0.88f));
        planetShader_.setVec3("uAccentColor", glm::vec3(0.15f, 0.55f, 1.0f));
        planetShader_.setInt("uBodyKind", 8);
        planetShader_.setBool("uIsSun", false);
        planetShader_.setBool("uSelected", false);
        spacecraftMesh_.draw();
    }
}

void Renderer::drawUi(const SolarSystem& system, const Camera& camera, const SimulationSettings& settings, int selectedIndex, const Spacecraft& spacecraft, UiOverlay& ui, int width, int height, float fps, bool showHelp) {
    ui.begin(width, height);
    const glm::vec4 panel(0.015f, 0.025f, 0.065f, 0.26f);
    const glm::vec4 border(0.15f, 0.40f, 0.65f, 0.24f);
    const glm::vec4 white(0.88f, 0.94f, 1.0f, 0.70f);
    const glm::vec4 muted(0.56f, 0.68f, 0.82f, 0.54f);
    const glm::vec4 accent(0.20f, 0.72f, 1.0f, 0.74f);

    ui.drawRect(12.0f, 12.0f, 354.0f, 232.0f, panel);
    ui.drawRect(12.0f, 12.0f, 354.0f, 1.0f, border);
    ui.drawText(24.0f, 24.0f, 2.0f, accent, "COSMOSIM 3D");
    ui.drawText(24.0f, 48.0f, 1.0f, muted, "INTERACTIVE SOLAR SYSTEM AND DEEP SPACE DEMO");
    ui.drawText(24.0f, 70.0f, 1.0f, white, "CAMERA  " + cameraModeName(camera.mode()));
    ui.drawText(24.0f, 84.0f, 1.0f, white, "DAY     " + fixed(settings.simulationDay, 1));
    ui.drawText(24.0f, 98.0f, 1.0f, white, "SPEED   " + fixed(settings.daysPerSecond, 1) + " DAYS/S");
    ui.drawText(24.0f, 112.0f, 1.0f, white, std::string("STATE   ") + (settings.paused ? "PAUSED" : "RUNNING"));
    ui.drawText(24.0f, 126.0f, 1.0f, white, "FPS     " + fixed(fps, 0));
    ui.drawText(24.0f, 148.0f, 1.0f, muted, "F1 MANUAL  F11 FULLSCREEN  SPACE PAUSE");
    ui.drawText(24.0f, 162.0f, 1.0f, muted, "1-8 PLANETS  M MOON  9 PLUTO  H BLACK HOLE");
    ui.drawText(24.0f, 176.0f, 1.0f, muted, "O ORBITS  L LABELS  N ATMOS  ARROWS CYCLE");
    ui.drawText(24.0f, 194.0f, 1.0f, accent, "SMARTER LABEL FILTER  CLEANER POPUP  MODERN PRESENTATION");
    const glm::vec4 creditBlue(0.35f, 0.80f, 1.0f, 0.95f);
    const glm::vec4 creditNameBlue(0.55f, 0.95f, 1.0f, 1.0f);
    ui.drawTextShadow(24.0f, 206.0f, 1.32f, creditBlue, "DEVELOPED BY ESTIUK ARAFAT ARNOB");
    ui.drawTextBold(24.0f, 206.0f, 1.32f, creditBlue, "DEVELOPED BY");
    ui.drawTextBold(24.0f + ui.textWidth("DEVELOPED BY ", 1.32f), 206.0f, 1.32f, creditNameBlue, "ESTIUK ARAFAT ARNOB");

    const float statusWidth = 340.0f;
    const float statusX = (static_cast<float>(width) - statusWidth) * 0.5f;
    ui.drawRect(statusX, 12.0f, statusWidth, 28.0f, glm::vec4(0.02f, 0.13f, 0.12f, 0.25f));
    ui.drawRect(statusX, 12.0f, statusWidth, 2.0f, glm::vec4(0.25f, 1.0f, 0.70f, 0.40f));
    ui.drawText(statusX + 16.0f, 20.0f, 1.10f, glm::vec4(0.72f, 1.0f, 0.88f, 0.64f), "V10 FINAL POLISH ACTIVE");

    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(system.bodies().size())) {
        const CelestialBody& selected = system.bodies()[static_cast<std::size_t>(selectedIndex)];
        const float panelWidth = 324.0f;
        const float panelX = static_cast<float>(width) - panelWidth - 12.0f;
        ui.drawRect(panelX, 12.0f, panelWidth, 248.0f, panel);
        ui.drawRect(panelX, 12.0f, panelWidth, 1.0f, border);
        const std::string parentName = selected.parentIndex >= 0 ? system.bodies()[static_cast<std::size_t>(selected.parentIndex)].name : "NONE";
        ui.drawText(panelX + 14.0f, 24.0f, 1.7f, white, selected.name);
        ui.drawText(panelX + 14.0f, 46.0f, 1.0f, muted, bodyKindName(selected.kind));
        ui.drawText(panelX + 14.0f, 66.0f, 1.0f, white, "PARENT       " + parentName);
        ui.drawText(panelX + 14.0f, 82.0f, 1.0f, white, "RADIUS       " + fixed(selected.radiusKm, 0) + " KM");
        ui.drawText(panelX + 14.0f, 98.0f, 1.0f, white, "MASS         " + fixed(selected.massEarths, 3) + " EARTH");
        ui.drawText(panelX + 14.0f, 114.0f, 1.0f, white, "GRAVITY      " + fixed(selected.gravityMs2, 2) + " M/S2");
        ui.drawText(panelX + 14.0f, 130.0f, 1.0f, white, "TEMPERATURE  " + fixed(selected.temperatureC, 0) + " C");
        ui.drawText(panelX + 14.0f, 146.0f, 1.0f, white, "ROTATION     " + fixed(selected.rotationPeriodHours, 1) + " HOURS");
        ui.drawText(panelX + 14.0f, 162.0f, 1.0f, white, "ORBIT        " + fixed(selected.orbit.orbitalPeriodDays, 1) + " DAYS");
        ui.drawText(panelX + 14.0f, 178.0f, 1.0f, white, "AXIAL TILT   " + fixed(selected.axialTiltDeg, 1) + " DEG");
        ui.drawText(panelX + 14.0f, 194.0f, 1.0f, white, "KNOWN MOONS  " + std::to_string(selected.knownMoons));
        ui.drawText(panelX + 14.0f, 220.0f, 1.0f, muted, "F FOLLOW  C CINEMA  T TOP VIEW  RIGHT/LEFT CYCLE");
    }

    const MissionTelemetry& mission = spacecraft.telemetry();
    if (mission.targetIndex >= 0 && mission.targetIndex < static_cast<int>(system.bodies().size())) {
        const float panelWidth = 360.0f;
        const float panelHeight = 104.0f;
        const float panelX = (static_cast<float>(width) - panelWidth) * 0.5f;
        const float panelY = static_cast<float>(height) - panelHeight - 14.0f;
        ui.drawRect(panelX, panelY, panelWidth, panelHeight, panel);
        const std::string targetName = system.bodies()[static_cast<std::size_t>(mission.targetIndex)].name;
        const std::string status = mission.active ? "MISSION TO " + targetName : (mission.completed ? "ARRIVED AT " + targetName : "MISSION READY");
        ui.drawText(panelX + 14.0f, panelY + 12.0f, 1.2f, accent, status);
        ui.drawBar(panelX + 14.0f, panelY + 36.0f, panelWidth - 28.0f, 10.0f, mission.progress, glm::vec4(0.08f, 0.12f, 0.20f, 0.28f), accent);
        ui.drawText(panelX + 14.0f, panelY + 54.0f, 1.0f, white, "PROGRESS " + fixed(mission.progress * 100.0f, 1) + "%   FUEL " + fixed(mission.fuelPercent, 1) + "%");
        ui.drawText(panelX + 14.0f, panelY + 70.0f, 1.0f, white, "SPEED " + fixed(mission.speedUnitsPerSecond, 2) + " U/S   REMAIN " + fixed(mission.remainingDistance, 2) + " U");
    }

    if (settings.showLabels) {
        struct LabelCandidate {
            int index = -1;
            bool selected = false;
            float depth = 0.0f;
            float x = 0.0f;
            float y = 0.0f;
            float width = 0.0f;
            float height = 0.0f;
            float scale = 1.0f;
            std::string text;
        };

        const glm::mat4 view = camera.viewMatrix();
        const float aspect = static_cast<float>(std::max(width, 1)) / static_cast<float>(std::max(height, 1));
        const glm::mat4 projection = glm::perspective(glm::radians(camera.zoomDegrees()), aspect, 0.01f, 950.0f);
        std::vector<LabelCandidate> labels;
        labels.reserve(system.bodies().size());

        for (std::size_t i = 0; i < system.bodies().size(); ++i) {
            const CelestialBody& current = system.bodies()[i];
            const glm::vec4 clip = projection * view * glm::vec4(current.worldPosition, 1.0f);
            if (clip.w <= 0.0f) continue;
            const glm::vec3 ndc = glm::vec3(clip) / clip.w;
            if (std::abs(ndc.x) > 1.08f || std::abs(ndc.y) > 1.08f || ndc.z < -1.0f || ndc.z > 1.0f) continue;

            LabelCandidate candidate;
            candidate.index = static_cast<int>(i);
            candidate.selected = static_cast<int>(i) == selectedIndex;
            candidate.depth = ndc.z;
            candidate.scale = candidate.selected ? 1.28f : 1.02f;
            candidate.text = current.name;
            candidate.width = ui.textWidth(candidate.text, candidate.scale) + 10.0f;
            candidate.height = 12.0f * candidate.scale;
            candidate.x = (ndc.x * 0.5f + 0.5f) * static_cast<float>(width) + 6.0f;
            candidate.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * static_cast<float>(height) - 8.0f;
            labels.push_back(candidate);
        }

        std::sort(labels.begin(), labels.end(), [](const LabelCandidate& a, const LabelCandidate& b) {
            if (a.selected != b.selected) return a.selected > b.selected;
            return a.depth < b.depth;
        });

        struct Rect {
            float x1, y1, x2, y2;
        };
        std::vector<Rect> placed;
        placed.reserve(labels.size());

        for (const LabelCandidate& candidate : labels) {
            const float margin = candidate.selected ? 2.0f : 6.0f;
            Rect currentRect {
                candidate.x - 4.0f - margin,
                candidate.y - 2.0f - margin,
                candidate.x - 4.0f + candidate.width + margin,
                candidate.y - 2.0f + candidate.height + margin
            };

            bool overlaps = false;
            for (const Rect& existing : placed) {
                if (!(currentRect.x2 < existing.x1 || currentRect.x1 > existing.x2 || currentRect.y2 < existing.y1 || currentRect.y1 > existing.y2)) {
                    overlaps = true;
                    break;
                }
            }

            if (overlaps && !candidate.selected) continue;
            placed.push_back(currentRect);

            const glm::vec4 labelColor = candidate.selected ? glm::vec4(0.90f, 0.96f, 1.0f, 0.76f) : glm::vec4(0.90f, 0.96f, 1.0f, 0.56f);
            const glm::vec4 boxColor = candidate.selected ? glm::vec4(0.04f, 0.12f, 0.24f, 0.18f) : glm::vec4(0.02f, 0.04f, 0.10f, 0.12f);
            ui.drawRect(candidate.x - 4.0f, candidate.y - 2.0f, candidate.width, candidate.height, boxColor);
            ui.drawRect(candidate.x - 4.0f, candidate.y - 2.0f, candidate.width, 1.0f, glm::vec4(0.18f, 0.55f, 0.95f, 0.22f));
            ui.drawText(candidate.x, candidate.y, candidate.scale, labelColor, candidate.text);
        }
    }

    if (showHelp) {
        const float helpWidth = 760.0f;
        const float helpHeight = 470.0f;
        const float helpX = (static_cast<float>(width) - helpWidth) * 0.5f;
        const float helpY = (static_cast<float>(height) - helpHeight) * 0.5f;
        ui.drawRect(helpX, helpY, helpWidth, helpHeight, glm::vec4(0.01f, 0.02f, 0.07f, 0.82f));
        ui.drawRect(helpX, helpY, helpWidth, 3.0f, glm::vec4(accent.r, accent.g, accent.b, 0.70f));
        ui.drawRect(helpX, helpY + helpHeight - 3.0f, helpWidth, 3.0f, glm::vec4(accent.r, accent.g, accent.b, 0.35f));
        ui.drawRect(helpX + helpWidth - 3.0f, helpY, 3.0f, helpHeight, glm::vec4(accent.r, accent.g, accent.b, 0.20f));
        ui.drawRect(helpX, helpY, 3.0f, helpHeight, glm::vec4(accent.r, accent.g, accent.b, 0.20f));
        ui.drawTextShadow(helpX + 20.0f, helpY + 16.0f, 2.1f, accent, "COSMOSIM 3D  |  CONTROLS & SHORTCUTS");
        ui.drawText(helpX + 20.0f, helpY + 48.0f, 1.0f, muted, "PRESS F1 TO SHOW OR HIDE THIS PANEL  |  F11 TOGGLES FULLSCREEN");
        ui.drawRect(helpX + 20.0f, helpY + 62.0f, helpWidth - 40.0f, 1.0f, glm::vec4(accent.r, accent.g, accent.b, 0.18f));

        // --- NAVIGATION ---
        ui.drawTextBold(helpX + 20.0f, helpY + 80.0f, 1.15f, accent, "[ CAMERA & NAVIGATION ]");
        ui.drawText(helpX + 28.0f, helpY + 100.0f, 1.12f, white,  "W A S D       Move camera forward / back / left / right");
        ui.drawText(helpX + 28.0f, helpY + 116.0f, 1.12f, white,  "Q / E         Move camera down / up");
        ui.drawText(helpX + 28.0f, helpY + 132.0f, 1.12f, white,  "SHIFT         Hold for speed boost");
        ui.drawText(helpX + 28.0f, helpY + 148.0f, 1.12f, white,  "MOUSE         Look around (right-click to capture or release)");
        ui.drawText(helpX + 28.0f, helpY + 164.0f, 1.12f, white,  "SCROLL WHEEL  Zoom in or out");
        ui.drawText(helpX + 28.0f, helpY + 180.0f, 1.12f, white,  "LEFT CLICK    Select a body when cursor is free");
        ui.drawRect(helpX + 20.0f, helpY + 196.0f, helpWidth - 40.0f, 1.0f, glm::vec4(accent.r, accent.g, accent.b, 0.12f));

        // --- SELECTION ---
        ui.drawTextBold(helpX + 20.0f, helpY + 204.0f, 1.15f, accent, "[ SELECT & FOLLOW ]");
        ui.drawText(helpX + 28.0f, helpY + 224.0f, 1.12f, white,  "0             Select the Sun");
        ui.drawText(helpX + 28.0f, helpY + 240.0f, 1.12f, white,  "1 - 8         Select Mercury through Neptune");
        ui.drawText(helpX + 28.0f, helpY + 256.0f, 1.12f, white,  "9 / M / H     Select Pluto / Moon / Black Hole");
        ui.drawText(helpX + 28.0f, helpY + 272.0f, 1.12f, white,  "LEFT / RIGHT  Cycle to previous or next body");
        ui.drawText(helpX + 28.0f, helpY + 288.0f, 1.12f, white,  "F             Follow selected body");
        ui.drawRect(helpX + 20.0f, helpY + 304.0f, helpWidth - 40.0f, 1.0f, glm::vec4(accent.r, accent.g, accent.b, 0.12f));

        // --- VIEW & SIM ---
        ui.drawTextBold(helpX + 20.0f, helpY + 312.0f, 1.15f, accent, "[ VIEW & SIMULATION ]");
        ui.drawText(helpX + 28.0f, helpY + 332.0f, 1.12f, white,  "T / C / V     Top-down / Cinematic / Free camera");
        ui.drawText(helpX + 28.0f, helpY + 348.0f, 1.12f, white,  "SPACE         Pause or resume simulation");
        ui.drawText(helpX + 28.0f, helpY + 364.0f, 1.12f, white,  "- / +         Slow down or speed up time");
        ui.drawText(helpX + 28.0f, helpY + 380.0f, 1.12f, white,  "O / L / B / N Toggle Orbits / Labels / Asteroids / Atmospheres");
        ui.drawText(helpX + 28.0f, helpY + 396.0f, 1.12f, white,  "G             Toggle educational scale");
        ui.drawText(helpX + 28.0f, helpY + 412.0f, 1.12f, white,  "J / X / R     Launch Mission / Abort / Reset epoch");
        ui.drawRect(helpX + 20.0f, helpY + 428.0f, helpWidth - 40.0f, 1.0f, glm::vec4(accent.r, accent.g, accent.b, 0.12f));

        // --- CREDIT ---
        const glm::vec4 creditBlue2(0.35f, 0.80f, 1.0f, 0.92f);
        const glm::vec4 creditNameBlue2(0.55f, 0.95f, 1.0f, 1.0f);
        ui.drawTextBold(helpX + 20.0f, helpY + 440.0f, 1.12f, creditBlue2, "DEVELOPED BY");
        ui.drawTextBold(helpX + 20.0f + ui.textWidth("DEVELOPED BY ", 1.12f), helpY + 440.0f, 1.12f, creditNameBlue2, "ESTIUK ARAFAT ARNOB");
        ui.drawText(helpX + 20.0f, helpY + 458.0f, 1.0f, muted, "F1 CLOSE THIS MENU     ESC EXIT APPLICATION");
    }

    const float cx = static_cast<float>(width) * 0.5f;
    const float cy = static_cast<float>(height) * 0.5f;
    ui.drawRect(cx - 7.0f, cy, 5.0f, 1.0f, glm::vec4(0.65f, 0.82f, 1.0f, 0.6f));
    ui.drawRect(cx + 2.0f, cy, 5.0f, 1.0f, glm::vec4(0.65f, 0.82f, 1.0f, 0.6f));
    ui.drawRect(cx, cy - 7.0f, 1.0f, 5.0f, glm::vec4(0.65f, 0.82f, 1.0f, 0.6f));
    ui.drawRect(cx, cy + 2.0f, 1.0f, 5.0f, glm::vec4(0.65f, 0.82f, 1.0f, 0.6f));

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ui.end(uiShader_);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

int Renderer::bodyKindId(int kind) {
    return kind;
}

} // namespace cosmosim
