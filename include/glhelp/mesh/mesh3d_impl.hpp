#pragma once

// 'classic' header-guard to avoid recursive definition
#ifndef REC_MESH3D_GUARD
#define REC_MESH3D_GUARD

#include <cmath>
#include <vector>

#include <glad/gl.h>

#include <glm/ext.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>

#include <glhelp/mesh/mesh3d.hpp>
#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

template< PositionProvider PositionSource >
Mesh3D< PositionSource >::Mesh3D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec3 >& vertices, const std::vector< GLuint >& indices, GLenum mode)
    : PositionSource(position_source), shader(std::move(shader)), mode(mode), vertex_count(vertices.size()), indices_count(indices.size())
{
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

template< PositionProvider PositionSource >
Mesh3D< PositionSource >::Mesh3D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec3 >& vertices, GLenum mode)
    : PositionSource(position_source), shader(std::move(shader)), mode(mode), vertex_count(vertices.size())
{
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

template< PositionProvider PositionSource >
Mesh3D< PositionSource >::Mesh3D(Mesh3D&& other) noexcept
    : shader(std::move(other.shader)), vao(other.vao), vbo(other.vbo), ebo(other.ebo), mode(other.mode), vertex_count(other.vertex_count), indices_count(other.indices_count)
{
  other.vao = 0;
  other.vbo = 0;
  other.ebo = 0;
}

template< PositionProvider PositionSource >
auto Mesh3D< PositionSource >::operator=(Mesh3D&& other) noexcept -> Mesh3D< PositionSource >&
{
  if (this == &other)
    return *this;

  if (ebo)
    glDeleteBuffers(1, &ebo);
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (vao)
    glDeleteVertexArrays(1, &vao);

  shader = std::move(other.shader);
  vao = other.vao;
  vbo = other.vbo;
  ebo = other.ebo;
  mode = other.mode;
  vertex_count = other.vertex_count;
  indices_count = other.indices_count;

  other.vao = 0;
  other.vbo = 0;
  other.ebo = 0;

  return *this;
}

template< PositionProvider PositionSource >
Mesh3D< PositionSource >::~Mesh3D()
{
  if (ebo)
    glDeleteBuffers(1, &ebo);
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (vao)
    glDeleteVertexArrays(1, &vao);
}

template< PositionProvider PositionSource >
void Mesh3D< PositionSource >::draw()
{
  glBindVertexArray(vao);

  shader->set_uniform("uModelTransform", get_model_matrix(*this));

  uniform_setter_callback();

  if (ebo) {
    glDrawElements(mode, indices_count, GL_UNSIGNED_INT, nullptr);
  }
  else {
    glDrawArrays(mode, 0, vertex_count);
  }
}

} // namespace glhelp

#endif
