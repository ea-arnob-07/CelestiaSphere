#include "Mesh.h"

#include <algorithm>
#include <utility>

#include <glm/gtc/type_ptr.hpp>

namespace cosmosim {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, GLenum primitive) {
    upload(vertices, indices, primitive);
}

Mesh::~Mesh() {
    reset();
}

Mesh::Mesh(Mesh&& other) noexcept
    : vao_(std::exchange(other.vao_, 0)),
      vbo_(std::exchange(other.vbo_, 0)),
      ebo_(std::exchange(other.ebo_, 0)),
      instanceVbo_(std::exchange(other.instanceVbo_, 0)),
      indexCount_(std::exchange(other.indexCount_, 0)),
      vertexCount_(std::exchange(other.vertexCount_, 0)),
      primitive_(other.primitive_),
      indexed_(other.indexed_),
      instanceCapacity_(other.instanceCapacity_) {}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        reset();
        vao_ = std::exchange(other.vao_, 0);
        vbo_ = std::exchange(other.vbo_, 0);
        ebo_ = std::exchange(other.ebo_, 0);
        instanceVbo_ = std::exchange(other.instanceVbo_, 0);
        indexCount_ = std::exchange(other.indexCount_, 0);
        vertexCount_ = std::exchange(other.vertexCount_, 0);
        primitive_ = other.primitive_;
        indexed_ = other.indexed_;
        instanceCapacity_ = other.instanceCapacity_;
    }
    return *this;
}

void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices, GLenum primitive) {
    reset();
    primitive_ = primitive;
    indexed_ = !indices.empty();
    vertexCount_ = static_cast<GLsizei>(vertices.size());
    indexCount_ = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STATIC_DRAW);

    if (indexed_) {
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)), indices.data(), GL_STATIC_DRAW);
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));
    glBindVertexArray(0);
}

void Mesh::uploadPositions(const std::vector<glm::vec3>& positions, GLenum primitive) {
    reset();
    primitive_ = primitive;
    indexed_ = false;
    vertexCount_ = static_cast<GLsizei>(positions.size());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(positions.size() * sizeof(glm::vec3)), positions.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glBindVertexArray(0);
}

void Mesh::setInstanceMatrices(const std::vector<glm::mat4>& matrices, GLenum usage) {
    if (vao_ == 0) return;
    if (instanceVbo_ == 0) glGenBuffers(1, &instanceVbo_);
    instanceCapacity_ = matrices.size();
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(matrices.size() * sizeof(glm::mat4)), matrices.data(), usage);
    for (GLuint i = 0; i < 4; ++i) {
        const GLuint location = 3 + i;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(location, 1);
    }
    glBindVertexArray(0);
}

void Mesh::updateInstanceMatrices(const std::vector<glm::mat4>& matrices) {
    if (instanceVbo_ == 0) {
        setInstanceMatrices(matrices, GL_DYNAMIC_DRAW);
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
    if (matrices.size() > instanceCapacity_) {
        instanceCapacity_ = matrices.size();
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(matrices.size() * sizeof(glm::mat4)), matrices.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(matrices.size() * sizeof(glm::mat4)), matrices.data());
    }
}

void Mesh::draw() const {
    glBindVertexArray(vao_);
    if (indexed_) {
        glDrawElements(primitive_, indexCount_, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(primitive_, 0, vertexCount_);
    }
    glBindVertexArray(0);
}

void Mesh::drawInstanced(GLsizei instanceCount) const {
    glBindVertexArray(vao_);
    if (indexed_) {
        glDrawElementsInstanced(primitive_, indexCount_, GL_UNSIGNED_INT, nullptr, instanceCount);
    } else {
        glDrawArraysInstanced(primitive_, 0, vertexCount_, instanceCount);
    }
    glBindVertexArray(0);
}

void Mesh::reset() {
    if (instanceVbo_ != 0) glDeleteBuffers(1, &instanceVbo_);
    if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = ebo_ = instanceVbo_ = 0;
    indexCount_ = vertexCount_ = 0;
    instanceCapacity_ = 0;
}

} // namespace cosmosim
