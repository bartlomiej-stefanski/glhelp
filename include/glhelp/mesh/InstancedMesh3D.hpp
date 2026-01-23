#pragma once

#include <array>
#include <cassert>
#include <vector>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <glhelp/Obj.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

// TODO: Add more possible types
template< typename T >
concept InstancableType = std::is_same_v< T, GLuint > || std::is_same_v< T, float > || std::is_same_v< T, glm::vec3 > || std::is_same_v< T, glm::vec4 > || std::is_same_v< T, glm::mat4 > || std::is_same_v< T, glm::mat3 >;

/// Allows for creation of instanced 3D meshes.
/// Each instance can have its own set of data provided as instance attributes.
/// The attributes will be available in the shader as per-instance vertex attributes,
/// first one will be located just after `Mesh3D` parameters.
/// TODO: Allow for multiple same types in InstancableTypes.
template< PositionProvider PositionSource, InstancableType... InstanceData >
class InstancedMesh3d : public Mesh3D< PositionSource > {
public:
  InstancedMesh3d(
      PositionSource position_source,
      std::shared_ptr< ShaderProgram > shader,
      const std::vector< glm::vec3 >& vertices,
      const std::vector< GLuint >& indices,
      GLenum mode,
      const std::tuple< std::vector< InstanceData >... >& instance_data);
  InstancedMesh3d(
      PositionSource position_source,
      std::shared_ptr< ShaderProgram > shader,
      const std::vector< glm::vec3 >& vertices,
      GLenum mode,
      const std::tuple< std::vector< InstanceData >... >& instance_data);

  template< glhelp::VertexType Vertex >
  InstancedMesh3d(
      PositionSource position_source,
      std::shared_ptr< ShaderProgram > shader,
      const glhelp::Obj< Vertex >& sphere,
      const std::tuple< std::vector< InstanceData >... >& instance_data);

  ~InstancedMesh3d() override;

  void update_buffers(const std::tuple< std::vector< InstanceData >... >& instance_data);

  template< InstancableType T >
    requires(std::is_same_v< T, InstanceData > || ...)
  void update_buffer_instance_data(const std::vector< T >& instance_data);

  void draw() override;

private:
  template< InstancableType T >
  void create_instance_data(GLuint vao, unsigned& start_inx, const std::vector< T >& instance_data);

protected:
  std::array< GLuint, sizeof...(InstanceData) > instance_vbo{};
  const unsigned instance_count;
};

} // namespace glhelp

// Include template ipmlementation
#include <glhelp/mesh/InstancedMesh3D_impl.hpp>
