#include <string>
#include <vector>

#include <glad/gl.h>

#include <glhelp/shader.hpp>

namespace glhelp {

ShaderProgram::ShaderProgram(const std::vector< GLuint >&& shaders) { program_id = link_program(std::move(shaders)); }

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept : program_id(other.program_id) { other.program_id = 0; }

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
  if (this != &other) {
    glDeleteProgram(program_id);
    program_id = other.program_id;
    other.program_id = 0;
  }
  return *this;
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(program_id); }

GLuint ShaderProgram::get_uniform_location(const std::string& name) const
{
  if (uniform_locations.find(name) != uniform_locations.end()) {
    return uniform_locations.at(name);
  }

  GLint result{glGetUniformLocation(program_id, name.c_str())};
  if (result != -1) [[likely]] {
    uniform_locations[name] = result;
    return result;
  }

  throw ShaderException("Uniform '" + name + "' not found");
}

void ShaderProgram::use() const
{
#ifdef DEBUG
  if (program_id == 0) [[unlikely]] {
    std::cerr << "Use of moved shader program! (acts as a no-op)" << std::endl;
  }
#endif

  glUseProgram(program_id);
}

GLuint create_shader(GLenum type, const std::string& source)
{
  GLuint shader{glCreateShader(type)};
  if (shader == 0) {
    throw ShaderException("Failed to create shader");
  }

  const char* source_ptr{source.c_str()};
  glShaderSource(shader, 1, &source_ptr, nullptr);
  glCompileShader(shader);

  GLint status{GL_FALSE};
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    GLint log_len{};
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

    std::string log(log_len, '\0');
    glGetShaderInfoLog(shader, log_len, nullptr, log.data());
    glDeleteShader(shader);
    throw ShaderException("Shader compilation failed: " + log);
  }

  return shader;
}

GLuint link_program(const std::vector< GLuint >&& shaders)
{
  GLuint program{glCreateProgram()};
  if (program == 0) {
    throw ShaderException("Failed to create program");
  }

  for (auto shader : shaders) {
    glAttachShader(program, shader);
  }

  glLinkProgram(program);

  GLint status{GL_FALSE};
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    GLint log_len{};
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

    std::string log(log_len, '\0');
    glGetProgramInfoLog(program, log_len, nullptr, log.data());
    for (auto shader : shaders) {
      glDetachShader(program, shader);
      glDeleteShader(shader);
    }

    glDeleteProgram(program);
    throw ShaderException("Program linking failed: " + log);
  }

  for (auto shader : shaders) {
    glDetachShader(program, shader);
    glDeleteShader(shader);
  }

  return program;
}

} // namespace glhelp
