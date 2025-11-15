#pragma once

#include <memory>

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <glhelp/Shader.hpp>
#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

/// General utility class for drawing simple 2D meshes.
/// It will pass the following uniforms to the ShaderProgram:
/// - uScale: Scale of the mesh.
/// - uPosition: Position of the mesh.
/// - uRotation: Rotation of the mesh.
/// - uAspectRatio: Aspect ratio of the mesh.
///
/// This class is meant to be used without a dedicated camera (only a simple 2D projection).
/// Origin point of the mesh is assumed to be located at (0, 0)
template< PositionProvider PositionSource >
class Mesh2D : public PositionSource {
public:
  /// Mesh will be drawn using glDrawElements with the specified mode
  Mesh2D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec2 >& vertices, const std::vector< GLuint >& indices, GLenum mode);
  /// Mesh will be drawn using glDrawArrays with the specified mode
  Mesh2D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec2 >& vertices, GLenum mode);

  // Copy constructors are removed as they will break OpenGL buffers.
  Mesh2D(const Mesh2D&) = delete;
  auto operator=(const Mesh2D&) -> Mesh2D& = delete;

  Mesh2D(Mesh2D&&) noexcept;
  auto operator=(Mesh2D&&) noexcept -> Mesh2D&;

  virtual ~Mesh2D();

  /// Draw the mesh using old transformation values
  void draw(GLfloat aspect_ratio);

  static auto calculate_rotation_matrix(GLfloat angle) -> glm::mat2;

  glm::vec2 scale{1.0f};

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

  auto get_rotation_matrix() -> glm::mat2;
};

/// Checks whether convex-hulls of a and b overlap.
/// Vericies are not stored in this class to save space. They can be stored in a class deriving from Mesh2D.
auto sat_overlap(const std::vector< glm::vec2 >& a, const std::vector< glm::vec2 >& b) -> bool;

} // namespace glhelp

// Include header implementation
#include <glhelp/mesh/Mesh2D_impl.hpp>
