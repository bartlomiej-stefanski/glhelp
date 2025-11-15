#pragma once

#include <array>
#include <cassert>
#include <glad/gl.h>

#include <glm/glm.hpp>

#include <glhelp/mesh/mesh3d.hpp>
#include <glhelp/position/PositionProvider.hpp>
#include <glm/gtc/random.hpp>
#include <vector>

namespace glhelp {

// TODO: Add more possible types
template< typename T >
concept InstancableType = std::is_same_v< T, float > || std::is_same_v< T, glm::vec3 >;

/// Allows for creation of instanced 3D meshes.
/// Each instance can have its own set of data provided as instance attributes.
/// The attributes will be available in the shader as per-instance vertex attributes,
/// first one will be located just after `Mesh3D` parameters.
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

  ~InstancedMesh3d() override;

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
#include <glhelp/mesh/InstancedMesh3d_impl.hpp>
