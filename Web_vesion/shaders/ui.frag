#version 330 core
in vec2 vTexCoord;
in vec4 vColor;
out vec4 FragColor;
uniform sampler2D uFont;
void main() {
    float alpha = texture(uFont, vTexCoord).r;
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
