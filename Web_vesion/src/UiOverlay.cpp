#include "UiOverlay.h"

#include <algorithm>
#include <cctype>
#include <cstddef>

#include "Shader.h"

namespace cosmosim {

namespace {
constexpr int kCellWidth = 8;
constexpr int kCellHeight = 8;
constexpr int kAtlasColumns = 16;
constexpr int kAtlasRows = 8;
constexpr int kAtlasWidth = kCellWidth * kAtlasColumns;
constexpr int kAtlasHeight = kCellHeight * kAtlasRows;
}

UiOverlay::~UiOverlay() {
    reset();
}

void UiOverlay::initialize() {
    reset();
    createGlyphPatterns();
    createFontTexture();
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(UiVertex) * 32768), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UiVertex), reinterpret_cast<void*>(offsetof(UiVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UiVertex), reinterpret_cast<void*>(offsetof(UiVertex, uv)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UiVertex), reinterpret_cast<void*>(offsetof(UiVertex, color)));
    glBindVertexArray(0);
    vertices_.reserve(32768);
}

void UiOverlay::begin(int width, int height) {
    viewportWidth_ = std::max(width, 1);
    viewportHeight_ = std::max(height, 1);
    vertices_.clear();
}

void UiOverlay::drawRect(float x, float y, float width, float height, const glm::vec4& color) {
    const glm::vec2 uv(0.5f / static_cast<float>(kAtlasWidth), 0.5f / static_cast<float>(kAtlasHeight));
    addQuad(x, y, x + width, y + height, uv, uv, color);
}

void UiOverlay::drawText(float x, float y, float scale, const glm::vec4& color, const std::string& text) {
    float cursorX = x;
    float cursorY = y;
    const float glyphWidth = 6.0f * scale;
    const float glyphHeight = 8.0f * scale;
    for (char rawCharacter : text) {
        const unsigned char raw = static_cast<unsigned char>(rawCharacter);
        if (raw == '\n') {
            cursorX = x;
            cursorY += glyphHeight;
            continue;
        }
        unsigned char character = raw;
        if (character >= 'a' && character <= 'z') {
            character = static_cast<unsigned char>(std::toupper(character));
        }
        if (character >= 128) character = '?';
        const int column = character % kAtlasColumns;
        const int row = character / kAtlasColumns;
        const glm::vec2 uv0(
            static_cast<float>(column * kCellWidth) / static_cast<float>(kAtlasWidth),
            static_cast<float>(row * kCellHeight) / static_cast<float>(kAtlasHeight));
        const glm::vec2 uv1(
            static_cast<float>(column * kCellWidth + 6) / static_cast<float>(kAtlasWidth),
            static_cast<float>(row * kCellHeight + 8) / static_cast<float>(kAtlasHeight));
        if (character != ' ') {
            addQuad(cursorX, cursorY, cursorX + glyphWidth, cursorY + glyphHeight, uv0, uv1, color);
        }
        cursorX += glyphWidth;
    }
}

void UiOverlay::drawTextBold(float x, float y, float scale, const glm::vec4& color, const std::string& text) {
    drawText(x + 1.0f, y, scale, color, text);
    drawText(x, y, scale, color, text);
}

void UiOverlay::drawTextShadow(float x, float y, float scale, const glm::vec4& color, const std::string& text) {
    const glm::vec4 shadow(0.0f, 0.0f, 0.0f, color.a * 0.6f);
    drawText(x + 1.0f, y + 1.0f, scale, shadow, text);
    drawText(x, y, scale, color, text);
}

void UiOverlay::drawBar(float x, float y, float width, float height, float value, const glm::vec4& background, const glm::vec4& foreground) {
    drawRect(x, y, width, height, background);
    drawRect(x + 1.0f, y + 1.0f, std::max(0.0f, width - 2.0f) * std::clamp(value, 0.0f, 1.0f), std::max(0.0f, height - 2.0f), foreground);
}

void UiOverlay::end(const Shader& shader) {
    if (vertices_.empty()) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertices_.size() * sizeof(UiVertex)), vertices_.data());

    shader.use();
    shader.setVec2("uViewport", glm::vec2(static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_)));
    shader.setInt("uFont", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
    glBindVertexArray(0);
}

float UiOverlay::textWidth(const std::string& text, float scale) const {
    std::size_t longest = 0;
    std::size_t current = 0;
    for (char character : text) {
        if (character == '\n') {
            longest = std::max(longest, current);
            current = 0;
        } else {
            ++current;
        }
    }
    longest = std::max(longest, current);
    return static_cast<float>(longest) * 6.0f * scale;
}

void UiOverlay::createGlyphPatterns() {
    glyphs_.fill({0, 0, 0, 0, 0, 0, 0});
    auto set = [&](char character, std::array<unsigned char, 7> rows) {
        glyphs_[static_cast<unsigned char>(character)] = rows;
    };

    set('A', {14,17,17,31,17,17,17}); set('B', {30,17,17,30,17,17,30});
    set('C', {14,17,16,16,16,17,14}); set('D', {30,17,17,17,17,17,30});
    set('E', {31,16,16,30,16,16,31}); set('F', {31,16,16,30,16,16,16});
    set('G', {14,17,16,23,17,17,15}); set('H', {17,17,17,31,17,17,17});
    set('I', {31,4,4,4,4,4,31}); set('J', {7,2,2,2,18,18,12});
    set('K', {17,18,20,24,20,18,17}); set('L', {16,16,16,16,16,16,31});
    set('M', {17,27,21,21,17,17,17}); set('N', {17,25,21,19,17,17,17});
    set('O', {14,17,17,17,17,17,14}); set('P', {30,17,17,30,16,16,16});
    set('Q', {14,17,17,17,21,18,13}); set('R', {30,17,17,30,20,18,17});
    set('S', {15,16,16,14,1,1,30}); set('T', {31,4,4,4,4,4,4});
    set('U', {17,17,17,17,17,17,14}); set('V', {17,17,17,17,17,10,4});
    set('W', {17,17,17,21,21,21,10}); set('X', {17,17,10,4,10,17,17});
    set('Y', {17,17,10,4,4,4,4}); set('Z', {31,1,2,4,8,16,31});

    set('0', {14,17,19,21,25,17,14}); set('1', {4,12,4,4,4,4,14});
    set('2', {14,17,1,2,4,8,31}); set('3', {30,1,1,14,1,1,30});
    set('4', {2,6,10,18,31,2,2}); set('5', {31,16,16,30,1,1,30});
    set('6', {14,16,16,30,17,17,14}); set('7', {31,1,2,4,8,8,8});
    set('8', {14,17,17,14,17,17,14}); set('9', {14,17,17,15,1,1,14});

    set('.', {0,0,0,0,0,6,6}); set(',', {0,0,0,0,6,6,4});
    set(':', {0,6,6,0,6,6,0}); set(';', {0,6,6,0,6,6,4});
    set('-', {0,0,0,31,0,0,0}); set('+', {0,4,4,31,4,4,0});
    set('/', {1,2,2,4,8,8,16}); set('\\', {16,8,8,4,2,2,1});
    set('(', {2,4,8,8,8,4,2}); set(')', {8,4,2,2,2,4,8});
    set('[', {14,8,8,8,8,8,14}); set(']', {14,2,2,2,2,2,14});
    set('=', {0,31,0,31,0,0,0}); set('_', {0,0,0,0,0,0,31});
    set('%', {17,2,4,8,16,17,0}); set('?', {14,17,1,2,4,0,4});
    set('!', {4,4,4,4,4,0,4}); set('*', {0,21,14,31,14,21,0});
    set('<', {2,4,8,16,8,4,2}); set('>', {8,4,2,1,2,4,8});
    set('|', {4,4,4,4,4,4,4}); set('#', {10,31,10,10,31,10,0});
}

void UiOverlay::createFontTexture() {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(kAtlasWidth * kAtlasHeight), 0);
    pixels[0] = 255;
    for (int character = 0; character < 128; ++character) {
        const int column = character % kAtlasColumns;
        const int row = character / kAtlasColumns;
        for (int y = 0; y < 7; ++y) {
            const unsigned char pattern = glyphs_[static_cast<std::size_t>(character)][static_cast<std::size_t>(y)];
            for (int x = 0; x < 5; ++x) {
                const bool on = (pattern & (1u << static_cast<unsigned int>(4 - x))) != 0;
                const int pixelX = column * kCellWidth + x;
                const int pixelY = row * kCellHeight + y;
                pixels[static_cast<std::size_t>(pixelY * kAtlasWidth + pixelX)] = on ? 255 : 0;
            }
        }
    }

    pixels[0] = 255;

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, kAtlasWidth, kAtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void UiOverlay::addQuad(float x0, float y0, float x1, float y1, glm::vec2 uv0, glm::vec2 uv1, const glm::vec4& color) {
    vertices_.push_back({{x0, y0}, {uv0.x, uv0.y}, color});
    vertices_.push_back({{x1, y0}, {uv1.x, uv0.y}, color});
    vertices_.push_back({{x1, y1}, {uv1.x, uv1.y}, color});
    vertices_.push_back({{x0, y0}, {uv0.x, uv0.y}, color});
    vertices_.push_back({{x1, y1}, {uv1.x, uv1.y}, color});
    vertices_.push_back({{x0, y1}, {uv0.x, uv1.y}, color});
}

void UiOverlay::reset() {
    if (texture_ != 0) glDeleteTextures(1, &texture_);
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    texture_ = vbo_ = vao_ = 0;
    vertices_.clear();
}

} // namespace cosmosim
