#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Mesh.h"
#include "SceneTypes.h"

namespace cosmosim::geometry {

Mesh makeUvSphere(unsigned int longitudeSegments = 64, unsigned int latitudeSegments = 32);
Mesh makeLowPolyRock(unsigned int seed = 1);
Mesh makeRing(unsigned int segments = 192);
Mesh makeSpacecraft();
Mesh makeOrbitLine(const OrbitalElements& orbit, float distanceScale, unsigned int segments = 256);
std::vector<glm::vec3> makeCirclePoints(float radius, unsigned int segments = 128);

} // namespace cosmosim::geometry
