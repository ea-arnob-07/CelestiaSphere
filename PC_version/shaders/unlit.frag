#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;
uniform vec4 uColor;
uniform bool uRadialFade;
void main() {
    float alpha = uColor.a;
    if (uRadialFade) {
        float edge = smoothstep(0.0, 0.10, vTexCoord.x) * (1.0 - smoothstep(0.88, 1.0, vTexCoord.x));
        float bands = 0.65 + 0.35 * sin(vTexCoord.x * 95.0 + vTexCoord.y * 4.0);
        alpha *= edge * bands;
    }
    FragColor = vec4(uColor.rgb, alpha);
}
