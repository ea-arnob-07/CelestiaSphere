#version 330 core
in vec4 vColor;
out vec4 FragColor;
void main() {
    vec2 centered = gl_PointCoord * 2.0 - 1.0;
    float radius = dot(centered, centered);
    if (radius > 1.0) discard;
    float alpha = (1.0 - radius) * vColor.a;
    FragColor = vec4(vColor.rgb * (1.1 + alpha), alpha);
}
