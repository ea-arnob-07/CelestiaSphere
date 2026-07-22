#version 330 core
in vec3 vNormal;
in vec3 vWorldPosition;
out vec4 FragColor;
uniform vec3 uSunPosition;
uniform vec3 uColor;
float hash31(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 31.32);
    return fract((p.x + p.y) * p.z);
}
void main() {
    vec3 normal = normalize(vNormal);
    vec3 lightDirection = normalize(uSunPosition - vWorldPosition);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    float grain = hash31(floor(vWorldPosition * 13.0));
    vec3 color = uColor * (0.32 + diffuse * 0.78) * (0.78 + grain * 0.35);
    FragColor = vec4(color, 1.0);
}
