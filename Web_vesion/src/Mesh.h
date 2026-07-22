#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace cosmosim {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Mesh {
public:
    Mesh() = default;
    Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, GLenum primitive = GL_TRIANGLES);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void upload(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, GLenum primitive = GL_TRIANGLES);
    void uploadPositions(const std::vector<glm::vec3>& positions, GLenum primitive);
    void setInstanceMatrices(const std::vector<glm::mat4>& matrices, GLenum usage = GL_DYNAMIC_DRAW);
    void updateInstanceMatrices(const std::vector<glm::mat4>& matrices);

    void draw() const;
    void drawInstanced(GLsizei instanceCount) const;
    bool valid() const { return vao_ != 0; }
    GLsizei indexCount() const { return indexCount_; }
    GLsizei vertexCount() const { return vertexCount_; }

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLuint instanceVbo_ = 0;
    GLsizei indexCount_ = 0;
    GLsizei vertexCount_ = 0;
    GLenum primitive_ = GL_TRIANGLES;
    bool indexed_ = false;
    std::size_t instanceCapacity_ = 0;

    void reset();
};

} // namespace cosmosim
