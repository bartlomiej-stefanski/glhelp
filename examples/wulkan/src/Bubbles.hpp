#pragma once

#include "obj_parser/Vertex.hpp"
#include <random>

#include <glm/glm.hpp>

#include <obj_parser/Obj.hpp>

#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/position/Position.hpp>

struct BubbleInfo {
  glm::vec3 position;
  union {
    glm::vec4 misceleanous;
    struct {
      float hsl_color;
      float time;
      float max_heigth;
      float start_height;
    };
  };
};

auto get_bubble_positions(unsigned n, glm::vec3 obstacle_scale, std::mt19937& dev) -> std::vector< glm::mat4 >;

class Boubbles : public glhelp::InstancedMesh3d< glhelp::SimplePosition, glm::mat4 > {
public:
  Boubbles(
    std::shared_ptr< glhelp::ShaderProgram > shader,
    glhelp::SimplePosition position,
    const obj_parser::Obj< obj_parser::VertexNormals >& sphere,
    std::vector< glm::mat4 > rombus_positions);

private:
  std::vector< glm::mat4 > bubble_positions;
};
