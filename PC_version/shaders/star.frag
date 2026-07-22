#version 330 core
in float vBrightness;
out vec4 FragColor;
void main() {
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float d = dot(p, p);
    if (d > 1.0) discard;

    float core = exp(-d * 8.5);
    float halo = exp(-d * 2.2);
    float sparkle = mix(0.85, 1.25, fract(vBrightness * 11.7));
    float alpha = clamp((core * 0.9 + halo * 0.35) * (0.25 + vBrightness * 1.35), 0.0, 1.0);

    vec3 cool = vec3(0.62, 0.76, 1.0);
    vec3 warm = vec3(1.0, 0.92, 0.76);
    vec3 neutral = vec3(0.92, 0.95, 1.0);
    float tintSelector = fract(vBrightness * 7.1 + 0.17);
    vec3 tint = mix(cool, neutral, smoothstep(0.18, 0.55, tintSelector));
    tint = mix(tint, warm, smoothstep(0.55, 0.95, tintSelector));

    FragColor = vec4(tint * (0.55 + sparkle * vBrightness), alpha);
}
