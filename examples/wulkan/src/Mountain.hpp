#pragma once

#include <glhelp/Obj.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>

#include <memory>
#include <vector>

#include <glm/glm.hpp>

auto get_mountain_heigth(float x, float z) -> float;

/// Returns a vector of vertices describing a plane.
auto get_mountain_vertices(unsigned n) -> std::vector< glhelp::VertexNormals >;

/// Returns a vector of indices describing a plane using vertices indices.
auto get_mountain_indices(unsigned n) -> std::vector< unsigned >;

class Mountain : public glhelp::Mesh3D< glhelp::SimplePosition > {
public:
  Mountain(glhelp::SimplePosition position, std::shared_ptr< glhelp::ShaderProgram > shader, unsigned n)
      : glhelp::Mesh3D< glhelp::SimplePosition >(
            position,
            std::move(shader),
            glhelp::Obj< glhelp::VertexNormals >(
                get_mountain_vertices(n),
                get_mountain_indices(n)))
  {
  }

  [[nodiscard]] auto get_wireframe_override() const noexcept -> bool override { return true; }

  [[nodiscard]] auto check_collision(glm::vec3 position, float radius) const -> bool;
};
