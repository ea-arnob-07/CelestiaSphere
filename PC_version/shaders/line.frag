#version 330 core
in float vDistance;
out vec4 FragColor;
uniform vec4 uColor;
uniform bool uDashed;
uniform float uTime;
void main() {
    if (uDashed && fract(vDistance * 0.55 - uTime * 0.08) > 0.60) discard;
    FragColor = uColor;
}
