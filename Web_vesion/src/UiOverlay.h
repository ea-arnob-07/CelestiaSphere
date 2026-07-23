#pragma once

#include <array>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace cosmosim {

class Shader;

class UiOverlay {
public:
    UiOverlay() = default;
    ~UiOverlay();

    UiOverlay(const UiOverlay&) = delete;
    UiOverlay& operator=(const UiOverlay&) = delete;

    void initialize();
    void begin(int width, int height);
    void drawRect(float x, float y, float width, float height, const glm::vec4& color);
    void drawText(float x, float y, float scale, const glm::vec4& color, const std::string& text);
    void drawTextBold(float x, float y, float scale, const glm::vec4& color, const std::string& text);
    void drawTextShadow(float x, float y, float scale, const glm::vec4& color, const std::string& text);
    void drawBar(float x, float y, float width, float height, float value, const glm::vec4& background, const glm::vec4& foreground);
    void end(const Shader& shader);

    float textWidth(const std::string& text, float scale) const;

private:
    struct UiVertex {
        glm::vec2 position;
        glm::vec2 uv;
        glm::vec4 color;
    };

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint texture_ = 0;
    int viewportWidth_ = 1;
    int viewportHeight_ = 1;
    std::vector<UiVertex> vertices_;
    std::array<std::array<unsigned char, 7>, 128> glyphs_{};

    void createGlyphPatterns();
    void createFontTexture();
    void addQuad(float x0, float y0, float x1, float y1, glm::vec2 uv0, glm::vec2 uv1, const glm::vec4& color);
    void reset();
};

} // namespace cosmosim
