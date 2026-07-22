#pragma once

#include <string>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace cosmosim {

class Shader {
public:
    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = {});
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool load(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = {});
    void use() const;
    GLuint id() const { return program_; }
    bool valid() const { return program_ != 0; }

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    GLuint program_ = 0;
    mutable std::unordered_map<std::string, GLint> uniformCache_;

    GLint uniformLocation(const std::string& name) const;
    static std::string readFile(const std::string& path);
    static GLuint compileStage(GLenum type, const std::string& source, const std::string& label);
    void reset();
};

} // namespace cosmosim
