#pragma once

#include <cstddef>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace cosmosim {

class Shader;

class Starfield {
public:
    Starfield() = default;
    ~Starfield();

    Starfield(const Starfield&) = delete;
    Starfield& operator=(const Starfield&) = delete;

    void initialize(std::size_t count = 4200);
    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float time) const;

private:
    struct StarVertex {
        glm::vec3 position;
        float brightness;
        float size;
        float phase;
    };

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLsizei count_ = 0;

    void reset();
};

} // namespace cosmosim
