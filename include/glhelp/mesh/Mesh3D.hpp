#pragma once

#include <memory>

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <obj_parser/Obj.hpp>

#include <glhelp/Shader.hpp>
#include <glhelp/mesh/Renderable.hpp>
#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

/// General utility class for drawing simple 3D meshes.
/// It will pass the following uniforms to the ShaderProgram:
/// - uModelTransform: Scale of the mesh.
///
/// Other uniforms should be read from common fields provided by UniformBuffer.
///
/// This class is meant to be used with a Camera class..
/// Origin point of the mesh is assumed to be located at (0, 0)
template< PositionProvider PositionSource >
class Mesh3D : public PositionSource, public Renderable {
public:
  /// Mesh will be drawn using glDrawElements with the specified mode
  Mesh3D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec3 >& vertices, const std::vector< GLuint >& indices, GLenum mode);
  /// Mesh will be drawn using glDrawArrays with the specified mode
  Mesh3D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const std::vector< glm::vec3 >& vertices, GLenum mode);
  /// Mesh will be loaded from `obj_parser::Obj` Object file
  template< obj_parser::VertexType Vertex >
  Mesh3D(PositionSource position_source, std::shared_ptr< ShaderProgram > shader, const obj_parser::Obj< Vertex >& obj);

  // Copy constructors are removed as they will break OpenGL buffers.
  Mesh3D(const Mesh3D&) = delete;
  auto operator=(const Mesh3D&) -> Mesh3D& = delete;

  Mesh3D(Mesh3D&&) noexcept;
  auto operator=(Mesh3D&&) noexcept -> Mesh3D&;

  virtual ~Mesh3D();

  void draw() override;
  [[nodiscard]] auto get_shader() const -> std::shared_ptr< ShaderProgram > override { return shader; }
  [[nodiscard]] auto get_id() const noexcept -> std::size_t override
  {
    return static_cast< std::size_t >(vao);
  }

protected:
  /// Number of layout parameters used by the mesh.
  unsigned layout_param_count{};

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
#include <glhelp/mesh/Mesh3D_impl.hpp>
