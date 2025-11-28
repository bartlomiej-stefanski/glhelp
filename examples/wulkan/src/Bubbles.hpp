#pragma once

#include "obj_parser/Vertex.hpp"
#include <random>

#include <glm/glm.hpp>

#include <obj_parser/Obj.hpp>

#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/position/Position.hpp>

using BubbleInfo_vec4 = glm::vec4;
union BubbleInfo {
  BubbleInfo_vec4 packed;
  struct {
    float hsl_color;
    float time;
    float max_heigth;
    float start_height;
  };
};

auto get_bubble_info(unsigned n, std::mt19937& rng) -> std::vector< BubbleInfo_vec4 >;
auto get_bubble_positions(unsigned n, std::mt19937& rng) -> std::vector< glm::vec3 >;

class Bubbles : public glhelp::InstancedMesh3d< glhelp::SimplePosition, glm::vec3, BubbleInfo_vec4 > {
public:
  Bubbles(
      std::shared_ptr< glhelp::ShaderProgram > shader,
      glhelp::SimplePosition position,
      const obj_parser::Obj< obj_parser::VertexNormals >& sphere,
      std::vector< glm::vec3 > bubble_positions,
      std::vector< BubbleInfo_vec4 > bubble_info);

private:
  std::vector< glm::vec3 > bubble_positions;
  std::vector< BubbleInfo > bubble_info;
};
