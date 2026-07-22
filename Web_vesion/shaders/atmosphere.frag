#version 330 core
in vec3 vWorldPosition;
in vec3 vNormal;
out vec4 FragColor;
uniform vec3 uCameraPosition;
uniform vec3 uAtmosphereColor;
uniform float uStrength;
void main() {
    vec3 viewDirection = normalize(uCameraPosition - vWorldPosition);
    float fresnel = pow(1.0 - max(dot(normalize(vNormal), viewDirection), 0.0), 2.2);
    float alpha = fresnel * (0.18 + uStrength * 0.85);
    FragColor = vec4(uAtmosphereColor * (0.65 + fresnel * 0.8), alpha);
}
