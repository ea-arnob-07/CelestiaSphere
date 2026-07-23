#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in float aBrightness;
layout (location = 2) in float aSize;
layout (location = 3) in float aPhase;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;
out float vBrightness;
out float vSize;
void main() {
    vec4 viewPosition = uView * vec4(aPosition, 1.0);
    gl_Position = uProjection * viewPosition;
    float dist = length(viewPosition.xyz);
    // Stars at close distance get bigger so they remain clearly visible as background
    float distScale = clamp(400.0 / dist, 0.6, 3.5);
    gl_PointSize = max(1.5, aSize * distScale);
    vBrightness = aBrightness * (0.82 + 0.18 * sin(uTime * 1.7 + aPhase));
    vSize = gl_PointSize;
}
