#pragma once

#include <obj_parser/Obj.hpp>

#include <glhelp/Shader.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>

#include <memory>
#include <vector>

#include <glm/glm.hpp>

auto get_mountain_heigth(float x, float z) -> float;

/// Returns a vector of vertices describing a plane.
auto get_mountain_vertices(unsigned n) -> std::vector< obj_parser::VertexNormals >;

/// Returns a vector of indices describing a plane using vertices indices.
auto get_mountain_indices(unsigned n) -> std::vector< unsigned >;

class Plane : public glhelp::Mesh3D< glhelp::SimplePosition > {
public:
  Plane(glhelp::SimplePosition position, std::shared_ptr< glhelp::ShaderProgram > shader, unsigned n)
      : glhelp::Mesh3D< glhelp::SimplePosition >(
            position,
            std::move(shader),
            obj_parser::Obj< obj_parser::VertexNormals >(
                get_mountain_vertices(n),
                get_mountain_indices(n)))
  {
  }
};
