#version 330 core
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;
uniform vec2 uViewport;
out vec2 vTexCoord;
out vec4 vColor;
void main() {
    vec2 ndc = vec2(aPosition.x / uViewport.x * 2.0 - 1.0, 1.0 - aPosition.y / uViewport.y * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
