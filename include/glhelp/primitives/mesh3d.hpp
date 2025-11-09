#pragma once

#include <memory>

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <glhelp/primitives/PositionProvider.hpp>
#include <glhelp/primitives/camera.hpp>
#include <glhelp/shader.hpp>

namespace glhelp {

/// General utility class for drawing simple 3D meshes.
/// It will pass the following uniforms to the ShaderProgram:
/// - uModelTransform: Scale of the mesh.
/// - uCameraTransform: View and Projection matrices combined.
///
/// This class is meant to be used with a Camera class..
/// Origin point of the mesh is assumed to be located at (0, 0)
template< PositionProvider PositionSource >
class Mesh3D : public PositionSource {
public:
  /// Mesh will be drawn using glDrawElements with the specified mode
  Mesh3D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec3 >& vertices, const std::vector< GLuint >& indices, GLenum mode);
  /// Mesh will be drawn using glDrawArrays with the specified mode
  Mesh3D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec3 >& vertices, GLenum mode);

  // Copy constructors are removed as they will break OpenGL buffers.
  Mesh3D(const Mesh3D&) = delete;
  Mesh3D& operator=(const Mesh3D&) = delete;

  Mesh3D(Mesh3D&&);
  Mesh3D& operator=(Mesh3D&&);

  virtual ~Mesh3D();

  /// Draw the mesh using old transformation values
  template< PositionProvider CameraPositionSource >
  void draw(const Camera< CameraPositionSource >& camera);

  glm::vec3 scale{1.0f};

protected:
  /// Called just before drawing the mesh.
  virtual void uniform_setter_callback() {}

  /// Reference to the shader program used for drawing the mesh.
  std::shared_ptr< ShaderProgram > shader;

  GLuint vao{}, vbo{}, ebo{};

  /// Mode that will be used for drawing the mesh
  GLenum mode;
  GLuint vertex_count;
  GLuint indices_count{};
};

} // namespace glhelp

// Include header implementation
#include <glhelp/primitives/mesh3d_impl.hpp>
