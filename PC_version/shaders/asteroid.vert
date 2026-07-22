#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 iModel0;
layout (location = 4) in vec4 iModel1;
layout (location = 5) in vec4 iModel2;
layout (location = 6) in vec4 iModel3;

uniform mat4 uView;
uniform mat4 uProjection;
out vec3 vNormal;
out vec3 vWorldPosition;

void main() {
    mat4 model = mat4(iModel0, iModel1, iModel2, iModel3);
    vec4 world = model * vec4(aPosition, 1.0);
    vWorldPosition = world.xyz;
    vNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
    gl_Position = uProjection * uView * world;
}
