#pragma once

// 'classic' header-guard to avoid recursive definition
#ifndef REC_INSTANCED_MESH3D_GUARD
#define REC_INSTANCED_MESH3D_GUARD

#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

template< PositionProvider PositionSource, InstancableType... InstanceData >
InstancedMesh3d< PositionSource, InstanceData... >::InstancedMesh3d(
    PositionSource position_source,
    std::shared_ptr< ShaderProgram > shader,
    const std::vector< glm::vec3 >& vertices,
    const std::vector< GLuint >& indices,
    GLenum mode,
    const std::tuple< std::vector< InstanceData >... >& instance_data)
    : Mesh3D< PositionSource >(position_source, std::move(shader), vertices, indices, mode),
      instance_count(static_cast< unsigned >(std::get< 0 >(instance_data).size()))
{
  // TODO: Figure out how to combine this with Mesh3D constructor where we already bind VAO.
  glBindVertexArray(this->vao);

  unsigned vbo_inx{}, start_inx{this->layout_param_count};
  (create_instance_data(vbo_inx++, start_inx, std::get< std::vector< InstanceData > >(instance_data)), ...);

  glBindVertexArray(0);
}

template< PositionProvider PositionSource, InstancableType... InstanceData >
InstancedMesh3d< PositionSource, InstanceData... >::InstancedMesh3d(
    PositionSource position_source,
    std::shared_ptr< ShaderProgram > shader,
    const std::vector< glm::vec3 >& vertices,
    GLenum mode,
    const std::tuple< std::vector< InstanceData >... >& instance_data)
    : Mesh3D< PositionSource >(position_source, std::move(shader), vertices, mode),
      instance_count(static_cast< unsigned >(std::get< 0 >(instance_data).size()))
{
  // TODO: Figure out how to combine this with Mesh3D constructor where we already bind VAO.
  glBindVertexArray(this->vao);

  unsigned vbo_inx{}, start_inx{this->layout_param_count};
  (create_instance_data(vbo_inx++, start_inx, std::get< std::vector< InstanceData > >(instance_data)), ...);

  glBindVertexArray(0);
}

template< PositionProvider PositionSource, InstancableType... InstanceData >
template< obj_parser::VertexType Vertex >
InstancedMesh3d< PositionSource, InstanceData... >::InstancedMesh3d(
    PositionSource position_source,
    std::shared_ptr< ShaderProgram > shader,
    const obj_parser::Obj< Vertex >& obj_data,
    const std::tuple< std::vector< InstanceData >... >& instance_data)
    : Mesh3D< PositionSource >(position_source, std::move(shader), obj_data),
      instance_count(static_cast< unsigned >(std::get< 0 >(instance_data).size()))
{
  // TODO: Figure out how to combine this with Mesh3D constructor where we already bind VAO.
  glBindVertexArray(this->vao);

  unsigned vbo_inx{}, start_inx{this->layout_param_count};
  (create_instance_data(vbo_inx++, start_inx, std::get< std::vector< InstanceData > >(instance_data)), ...);

  glBindVertexArray(0);
}

template< PositionProvider PositionSource, InstancableType... InstanceData >
InstancedMesh3d< PositionSource, InstanceData... >::~InstancedMesh3d()
{
  for (const auto& vbo : instance_vbo) {
    if (vbo)
      glDeleteBuffers(1, &vbo);
  }
}

template< PositionProvider PositionSource, InstancableType... InstanceData >
template< InstancableType T >
void InstancedMesh3d< PositionSource, InstanceData... >::create_instance_data(unsigned vbo_inx, unsigned& start_inx, const std::vector< T >& instance_data)
{
  if constexpr (std::is_same_v< T, float >) {
    glEnableVertexAttribArray(start_inx);

    glGenBuffers(1, &instance_vbo[vbo_inx]);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[vbo_inx]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * instance_data.size(), instance_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(start_inx, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(start_inx, 1);
    start_inx++;
  }
  else if constexpr (std::is_same_v< T, glm::vec3 > || std::is_same_v< T, glm::vec4 >) {
    glEnableVertexAttribArray(start_inx);

    glGenBuffers(1, &instance_vbo[vbo_inx]);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[vbo_inx]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(T) * instance_data.size(), instance_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(start_inx, sizeof(T) / sizeof(float), GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(start_inx, 1);
    start_inx++;
  }
  else if constexpr (std::is_same_v< T, glm::mat4 >) {
    glGenBuffers(1, &instance_vbo[vbo_inx]);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[vbo_inx]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * instance_data.size(), instance_data.data(), GL_STATIC_DRAW);

    for (unsigned i = 0; i < 4; i++) {
      glEnableVertexAttribArray(start_inx + i);
      glVertexAttribPointer(start_inx + i, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), reinterpret_cast< void* >(sizeof(glm::vec4) * i));
      glVertexAttribDivisor(start_inx + i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    start_inx += 4;
  }
  else {
    static_assert(false, "Unsupported instance data type");
  }
}

template< PositionProvider PositionSource, InstancableType... InstanceData >
void InstancedMesh3d< PositionSource, InstanceData... >::draw()
{
  glBindVertexArray(this->vao);

  const auto model_matrix{get_model_matrix(*this)};
  this->shader->set_uniform("uModelTransform", model_matrix);

  const auto normal_transform{this->shader->uniform_location("uNormalTransform")};
  if (normal_transform.has_value()) [[likely]] {
    const auto normal_transform{glm::mat3{glm::transpose(glm::inverse(model_matrix))}};
    this->shader->set_uniform("uNormalTransform", normal_transform);
  }

  this->uniform_setter_callback();

  if (this->ebo) {
    glDrawElementsInstanced(this->mode, this->indices_count, GL_UNSIGNED_INT, nullptr, instance_count);
  }
  else {
    glDrawArraysInstanced(this->mode, 0, this->vertex_count, instance_count);
  }

  glBindVertexArray(0);
}

} // namespace glhelp

#endif
