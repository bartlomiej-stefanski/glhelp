#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glhelp/shader_helpers.hpp>

namespace glhelp {

template< class T >
concept UniformType =
    std::is_same_v< T, bool > || std::is_same_v< T, int > || std::is_same_v< T, float > || std::is_same_v< T, glm::vec2 > || std::is_same_v< T, glm::vec3 > || std::is_same_v< T, glm::vec4 > || std::is_same_v< T, glm::mat2 > || std::is_same_v< T, glm::mat3 > || std::is_same_v< T, glm::mat4 >;

class ShaderException : public std::runtime_error {
public:
  ShaderException(const std::string& message) : std::runtime_error(message) {}
};

/// RAII-style shader program object.
class ShaderProgram {
public:
  ShaderProgram(const std::vector< GLuint >&& shaders);
  ~ShaderProgram();

  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;

  ShaderProgram(ShaderProgram&& other) noexcept;
  ShaderProgram& operator=(ShaderProgram&& other) noexcept;

  GLuint id() const noexcept { return program_id; }

  /// Get the location of a uniform variable in the shader program.
  /// Throws ShaderException if the uniform variable is not found.
  GLuint get_uniform_location(const std::string& name) const;

  /// Set the value of a uniform variable in the shader program.
  /// Throws ShaderException if the uniform variable is not found.
  template< UniformType T >
  void set_uniform(const std::string& name, const T& value) const;

  /// Use this shader program for subsequent rendering operations.
  void use() const;

private:
  GLuint program_id;

  mutable std::unordered_map< std::string, GLint > uniform_locations;
};

/// Creates a shader object of the given type by compiling the provided source code.
GLuint create_shader(GLenum type, const std::string& source);

/// Creates a shader program object by linking the provided shader objects.
/// Provided shaders are consumed by the function (as in should not be accessed after this call).
GLuint link_program(const std::vector< GLuint >&& shaders);

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
