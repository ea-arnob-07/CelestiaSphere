#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in float aBrightness;
layout (location = 2) in float aSize;
layout (location = 3) in float aPhase;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;
out float vBrightness;
void main() {
    vec4 viewPosition = uView * vec4(aPosition, 1.0);
    gl_Position = uProjection * viewPosition;
    gl_PointSize = aSize;
    vBrightness = aBrightness * (0.78 + 0.22 * sin(uTime * 1.7 + aPhase));
}
