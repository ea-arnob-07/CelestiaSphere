#version 330 core
layout (location = 0) in vec3 aPosition;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out float vDistance;
void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    vDistance = length(world.xyz);
    gl_Position = uProjection * uView * world;
}
