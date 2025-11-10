#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glhelp/utils/shader_helpers.hpp>

namespace glhelp {

template< class T >
concept UniformType =
    std::is_same_v< T, bool > || std::is_same_v< T, int > || std::is_same_v< T, float > || std::is_same_v< T, glm::vec2 > || std::is_same_v< T, glm::vec3 > || std::is_same_v< T, glm::vec4 > || std::is_same_v< T, glm::mat2 > || std::is_same_v< T, glm::mat3 > || std::is_same_v< T, glm::mat4 >;

class ShaderException : public std::runtime_error {
public:
  ShaderException(const std::string& message) : std::runtime_error(message) {}
};

template< typename T >
class UniformBuffer {
public:
  UniformBuffer(int draw_type)
  {
    glGenBuffers(1, &id);
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, draw_type);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    index = global_index++;
    glBindBufferRange(GL_UNIFORM_BUFFER, index, id, 0, sizeof(T));
  }

  ~UniformBuffer()
  {
    glDeleteBuffers(1, &id);
  }

  void update_buffer(const T& data) const
  {
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  [[nodiscard]] auto get_id() const noexcept -> GLuint { return id; }
  [[nodiscard]] auto get_index() const noexcept -> GLuint { return index; }

private:
  GLuint id;
  GLuint index;
  inline static GLuint global_index{};
};

/// RAII-style shader program object.
class ShaderProgram {
public:
  ShaderProgram(std::vector< GLuint >&& shaders);
  ~ShaderProgram();

  ShaderProgram(const ShaderProgram&) = delete;
  auto operator=(const ShaderProgram&) -> ShaderProgram& = delete;

  ShaderProgram(ShaderProgram&& other) noexcept;
  auto operator=(ShaderProgram&& other) noexcept -> ShaderProgram&;

  auto id() const noexcept -> GLuint { return program_id; }

  /// Get the location of a uniform variable in the shader program.
  /// @throws ShaderException if the uniform variable is not found.
  auto get_uniform_location(const std::string& name) const -> GLuint;

  /// Set the value of a uniform variable in the shader program.
  /// @throws ShaderException if the uniform variable is not found.
  template< UniformType T >
  void set_uniform(const std::string& name, const T& value) const;

  /// Use this shader program for subsequent rendering operations.
  void use() const;

  auto operator==(const ShaderProgram& other) const noexcept -> bool
  {
    return program_id == other.program_id;
  }

  struct CommonData {
    glm::mat4 camera_matrix;
    float time;
  };

  inline static std::optional< UniformBuffer< CommonData > > common_data{};

private:
  GLuint program_id;

  mutable std::unordered_map< std::string, GLint > uniform_locations;
};

/// Creates a shader object of the given type by compiling the provided source code.
auto create_shader(GLenum type, const std::string& source) -> GLuint;

/// Creates a shader object of the given type by reading the source from file and compiling the code.
auto create_shader_from_file(GLenum type, const std::string& file_path) -> GLuint;

/// Creates a shader program object by linking the provided shader objects.
/// Provided shaders are consumed by the function (as in should not be accessed after this call).
auto link_program(const std::vector< GLuint >&& shaders) -> GLuint;

template< UniformType T >
void ShaderProgram::set_uniform(const std::string& name, const T& value) const
{
  auto location{get_uniform_location(name)};

  if constexpr (std::is_same_v< T, bool >) {
    glUniform1i(location, value ? 1 : 0);
  }
  else if constexpr (std::is_same_v< T, int >) {
    glUniform1i(location, value);
  }
  else if constexpr (std::is_same_v< T, float >) {
    glUniform1f(location, value);
  }
  else if constexpr (std::is_same_v< T, glm::vec2 >) {
    glUniform2fv(location, 1, glm::value_ptr(value));
  }
  else if constexpr (std::is_same_v< T, glm::vec3 >) {
    glUniform3fv(location, 1, glm::value_ptr(value));
  }
  else if constexpr (std::is_same_v< T, glm::vec4 >) {
    glUniform4fv(location, 1, glm::value_ptr(value));
  }
  else if constexpr (std::is_same_v< T, glm::mat2 >) {
    glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }
  else if constexpr (std::is_same_v< T, glm::mat3 >) {
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }
  else if constexpr (std::is_same_v< T, glm::mat4 >) {
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }
  else {
    static_assert(std::is_same_v< T, bool >, "Unsupported uniform type");
  }
}

} // namespace glhelp

template<>
struct std::hash< glhelp::ShaderProgram > {
  auto operator()(const glhelp::ShaderProgram& shader_program) const noexcept -> std::size_t
  {
    return shader_program.id();
  }
};
