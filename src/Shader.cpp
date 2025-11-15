#include <fstream>
#include <string>
#include <vector>

#include <glad/gl.h>

#include <glhelp/Shader.hpp>

namespace glhelp {

ShaderProgram::ShaderProgram(std::vector< GLuint >&& shaders)
{
  if (!common_data.has_value()) {
    common_data.emplace(GL_DYNAMIC_DRAW);
  }

  program_id = link_program(std::move(shaders));
  GLuint common_buffer{glGetUniformBlockIndex(program_id, "lCommon")};
  if (common_buffer != GL_INVALID_INDEX) {
    glUniformBlockBinding(program_id, common_buffer, common_data.value().get_index());
  }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept : program_id(other.program_id) { other.program_id = 0; }

auto ShaderProgram::operator=(ShaderProgram&& other) noexcept -> ShaderProgram&
{
  if (this != &other) {
    glDeleteProgram(program_id);
    program_id = other.program_id;
    other.program_id = 0;
  }
  return *this;
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(program_id); }

auto ShaderProgram::get_uniform_location(const std::string& name) const -> GLuint
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

auto create_shader(GLenum type, const std::string& source) -> GLuint
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

auto create_shader_from_file(GLenum type, const std::string& file_path) -> GLuint
{
  std::ifstream file_stream(file_path);
  if (!file_stream.is_open()) {
    throw ShaderException("Failed to open shader file: " + file_path);
  }

  std::string source((std::istreambuf_iterator< char >(file_stream)), std::istreambuf_iterator< char >());
  return create_shader(type, source);
}

auto link_program(const std::vector< GLuint >&& shaders) -> GLuint
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
