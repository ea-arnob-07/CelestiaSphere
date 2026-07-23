#version 330 core
in float vBrightness;
in float vSize;
out vec4 FragColor;
void main() {
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float d = dot(p, p);
    if (d > 1.0) discard;

    // Tight bright core + soft halo + very faint outer glow (diffraction-like)
    float core    = exp(-d * 10.0);
    float halo    = exp(-d * 2.8);
    float outerGlow = exp(-d * 0.9) * 0.18;

    // Brighter stars get a larger, more visible halo
    float haloStrength = 0.28 + vBrightness * 0.55;
    float alpha = clamp(
        core * 1.0 + halo * haloStrength + outerGlow,
        0.0, 1.0
    ) * (0.20 + vBrightness * 1.55);
    alpha = clamp(alpha, 0.0, 1.0);

    // Realistic stellar color temperature: blue giants, yellow dwarfs, red giants
    vec3 blueStar   = vec3(0.68, 0.82, 1.00);  // O/B type
    vec3 whiteStar  = vec3(0.94, 0.96, 1.00);  // A/F type
    vec3 yellowStar = vec3(1.00, 0.96, 0.78);  // G type (like Sun)
    vec3 orangeStar = vec3(1.00, 0.80, 0.52);  // K type
    float tintSel = fract(vBrightness * 7.3 + 0.13);
    vec3 tint;
    if (tintSel < 0.28) {
        tint = mix(blueStar, whiteStar, tintSel / 0.28);
    } else if (tintSel < 0.62) {
        tint = mix(whiteStar, yellowStar, (tintSel - 0.28) / 0.34);
    } else {
        tint = mix(yellowStar, orangeStar, (tintSel - 0.62) / 0.38);
    }

    FragColor = vec4(tint, alpha);
}
