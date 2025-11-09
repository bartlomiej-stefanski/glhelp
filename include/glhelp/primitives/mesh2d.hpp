#pragma once

#include <memory>

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <glhelp/primitives/PositionProvider.hpp>
#include <glhelp/shader.hpp>

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
  Mesh2D(std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec2 >& vertices, const std::vector< GLuint >& indices, GLenum mode);
  /// Mesh will be drawn using glDrawArrays with the specified mode
  Mesh2D(std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec2 >& vertices, GLenum mode);

  /// Copy and move constructors are deleted because moving the location of
  /// opengl buffer holder may invalidate the buffer object.
  Mesh2D(const Mesh2D&) = delete;
  Mesh2D& operator=(const Mesh2D&) = delete;
  Mesh2D(Mesh2D&&) = delete;
  Mesh2D& operator=(Mesh2D&&) = delete;

  virtual ~Mesh2D();

  /// Draw the mesh using old transformation values
  void draw(GLfloat aspect_ratio);

  static glm::mat2 calculate_rotation_matrix(GLfloat angle);

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

  glm::mat2 get_rotation_matrix();
};

/// Checks whether convex-hulls of a and b overlap.
/// Vericies are not stored in this class to save space. They can be stored in a class deriving from Mesh2D.
bool sat_overlap(const std::vector< glm::vec2 >& a, const std::vector< glm::vec2 >& b);

} // namespace glhelp

// Include header implementation
#include <glhelp/primitives/mesh2d_impl.hpp>
