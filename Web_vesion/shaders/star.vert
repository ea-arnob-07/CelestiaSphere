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
    // Scale point size so stars far away appear smaller but never disappear below 1px
    float distScale = clamp(300.0 / dist, 0.55, 2.8);
    gl_PointSize = max(0.9, aSize * distScale);
    vBrightness = aBrightness * (0.80 + 0.20 * sin(uTime * 1.7 + aPhase));
    vSize = gl_PointSize;
}
