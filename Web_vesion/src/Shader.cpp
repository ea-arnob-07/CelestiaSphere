#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace cosmosim {

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    load(vertexPath, fragmentPath, geometryPath);
}

Shader::~Shader() {
    reset();
}

Shader::Shader(Shader&& other) noexcept
    : program_(std::exchange(other.program_, 0)), uniformCache_(std::move(other.uniformCache_)) {}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        reset();
        program_ = std::exchange(other.program_, 0);
        uniformCache_ = std::move(other.uniformCache_);
    }
    return *this;
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    reset();
    uniformCache_.clear();

    const std::string vertexSource = readFile(vertexPath);
    const std::string fragmentSource = readFile(fragmentPath);
    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }

    const GLuint vertex = compileStage(GL_VERTEX_SHADER, vertexSource, vertexPath);
    const GLuint fragment = compileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
    GLuint geometry = 0;
    if (!geometryPath.empty()) {
        const std::string geometrySource = readFile(geometryPath);
        if (geometrySource.empty()) {
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            return false;
        }
        geometry = compileStage(GL_GEOMETRY_SHADER, geometrySource, geometryPath);
    }

    if (vertex == 0 || fragment == 0 || (!geometryPath.empty() && geometry == 0)) {
        if (vertex != 0) glDeleteShader(vertex);
        if (fragment != 0) glDeleteShader(fragment);
        if (geometry != 0) glDeleteShader(geometry);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vertex);
    glAttachShader(program_, fragment);
    if (geometry != 0) glAttachShader(program_, geometry);
    glLinkProgram(program_);

    GLint success = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<std::size_t>(std::max(logLength, 1)));
        glGetProgramInfoLog(program_, logLength, nullptr, log.data());
        std::cerr << "Shader link failure:\n" << log.data() << '\n';
        reset();
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry != 0) glDeleteShader(geometry);
    return program_ != 0;
}

void Shader::use() const {
    glUseProgram(program_);
}

void Shader::setBool(const std::string& name, bool value) const { glUniform1i(uniformLocation(name), value ? 1 : 0); }
void Shader::setInt(const std::string& name, int value) const { glUniform1i(uniformLocation(name), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(uniformLocation(name), value); }
void Shader::setVec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(uniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::setVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(uniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::setVec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(uniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::setMat3(const std::string& name, const glm::mat3& value) const { glUniformMatrix3fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }
void Shader::setMat4(const std::string& name, const glm::mat4& value) const { glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }

GLint Shader::uniformLocation(const std::string& name) const {
    const auto found = uniformCache_.find(name);
    if (found != uniformCache_.end()) {
        return found->second;
    }
    const GLint location = glGetUniformLocation(program_, name.c_str());
    uniformCache_[name] = location;
    return location;
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open shader: " << path << '\n';
        return {};
    }
    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

GLuint Shader::compileStage(GLenum type, const std::string& source, const std::string& label) {
    std::string finalSource = source;
#ifdef __EMSCRIPTEN__
    size_t pos = finalSource.find("#version 330 core");
    if (pos != std::string::npos) {
        finalSource.replace(pos, 17, "#version 300 es\nprecision highp float;");
    }
#endif

    const GLuint shader = glCreateShader(type);
    const char* pointer = finalSource.c_str();
    glShaderSource(shader, 1, &pointer, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<std::size_t>(std::max(logLength, 1)));
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::cerr << "Shader compile failure in " << label << ":\n" << log.data() << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Shader::reset() {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

} // namespace cosmosim
