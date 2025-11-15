#pragma once

// 'classic' header-guard to avoid recursive definition
#ifndef REC_MESH2D_GUARD
#define REC_MESH2D_GUARD

#include <cmath>
#include <vector>

#include <glad/gl.h>

#include <glm/ext.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glhelp/mesh/Mesh2D.hpp>
#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

template< PositionProvider PositionSource >
Mesh2D< PositionSource >::Mesh2D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec2 >& vertices, const std::vector< GLuint >& indices, GLenum mode)
    : PositionSource(position_source), shader(std::move(shader)), mode(mode), vertex_count(vertices.size()), indices_count(indices.size())
{
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

template< PositionProvider PositionSource >
Mesh2D< PositionSource >::Mesh2D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec2 >& vertices, GLenum mode)
    : PositionSource(position_source), shader(std::move(shader)), mode(mode), vertex_count(vertices.size())
{
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

template< PositionProvider PositionSource >
Mesh2D< PositionSource >::Mesh2D(Mesh2D&& other) noexcept
    : scale(other.scale), shader(std::move(other.shader)), vao(other.vao), vbo(other.vbo), ebo(other.ebo), mode(other.mode), vertex_count(other.vertex_count), indices_count(other.indices_count)
{
  other.vao = 0;
  other.vbo = 0;
  other.ebo = 0;
}

template< PositionProvider PositionSource >
auto Mesh2D< PositionSource >::operator=(Mesh2D&& other) noexcept -> Mesh2D< PositionSource >&
{
  if (this == &other)
    return *this;

  if (ebo)
    glDeleteBuffers(1, &ebo);
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (vao)
    glDeleteVertexArrays(1, &vao);

  scale = other.scale;
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
Mesh2D< PositionSource >::~Mesh2D()
{
  if (ebo)
    glDeleteBuffers(1, &ebo);
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (vao)
    glDeleteVertexArrays(1, &vao);
}

template< PositionProvider PositionSource >
void Mesh2D< PositionSource >::draw(GLfloat aspect_ratio)
{
  shader->use();
  glBindVertexArray(vao);

  shader->set_uniform("uScale", scale);
  shader->set_uniform("uRotation", get_rotation_matrix());
  shader->set_uniform< glm::vec2 >("uTranslation", this->get_position());
  shader->set_uniform("uAspectRatio", aspect_ratio);

  uniform_setter_callback();

  if (ebo) {
    glDrawElements(mode, indices_count, GL_UNSIGNED_INT, nullptr);
  }
  else {
    glDrawArrays(mode, 0, vertex_count);
  }
}

template< PositionProvider PositionSource >
auto Mesh2D< PositionSource >::calculate_rotation_matrix(GLfloat angle) -> glm::mat2
{
  return glm::mat2{
      glm::vec2{glm::cos(angle), glm::sin(angle)},
      glm::vec2{-glm::sin(angle), glm::cos(angle)}};
}

template< PositionProvider PositionSource >
auto Mesh2D< PositionSource >::get_rotation_matrix() -> glm::mat2
{
  const glm::vec3 euler_angles{glm::eulerAngles(this->get_rotation())};
  return calculate_rotation_matrix(euler_angles.z);
}

inline auto sat_overlap(const std::vector< glm::vec2 >& a, const std::vector< glm::vec2 >& b) -> bool
{
  auto edge_normal = [](const glm::vec2& a, const glm::vec2& b) {
    glm::vec2 edge{b - a};
    return glm::normalize(glm::vec2{-edge.y, edge.x});
  };

  auto project = [](const std::vector< glm::vec2 >& points, glm::vec2 normal) {
    float min{INFINITY}, max{-INFINITY};
    for (const auto& p : points) {
      min = std::min(min, glm::dot(p, normal));
      max = std::max(max, glm::dot(p, normal));
    }
    return std::make_pair(min, max);
  };

  auto detect_overlap = [&](const auto& convex_hull) {
    for (unsigned i = 0; i < convex_hull.size(); i++) {
      const glm::vec2 axis{edge_normal(convex_hull[i], convex_hull[(i + 1) % convex_hull.size()])};
      auto [a_min, a_max]{project(a, axis)};
      auto [b_min, b_max]{project(b, axis)};
      if (a_min > b_max || b_min > a_max)
        return false;
    }
    return true;
  };

  return detect_overlap(a) && detect_overlap(b);
}

} // namespace glhelp

#endif
