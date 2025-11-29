#pragma once

#include <random>

#include <glm/glm.hpp>

#include <obj_parser/Obj.hpp>

#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/position/Position.hpp>

using BubbleData_mat3 = glm::mat3;

struct BubbleInfo {
  union {
    glm::vec3 _padding1;
    struct {
      float hsl_color;
      float time_offset;
      float _fpadd1;
    };
  };
  union {
    glm::vec3 _padding2;
    struct {
      float max_heigth;
      float start_height;
      float _fpadd2;
    };
  };
};

union BubbleData {
  BubbleData_mat3 packed;
  struct Data {
    glm::vec3 position;
    BubbleInfo info;
  } data;
};

auto get_bubble_info(unsigned n, std::mt19937& rng) -> std::vector< BubbleInfo >;
auto get_bubble_positions(unsigned n, std::mt19937& rng) -> std::vector< glm::vec3 >;

auto get_bubble_data(unsigned n, std::mt19937& rng) -> std::vector< BubbleData >;

class Bubbles : public glhelp::InstancedMesh3d< glhelp::SimplePosition, BubbleData_mat3 > {
public:
  Bubbles(
      std::shared_ptr< glhelp::ShaderProgram > shader,
      glhelp::SimplePosition position,
      const obj_parser::Obj< obj_parser::VertexNormals >& sphere,
      std::vector< BubbleData > bubble_data);

  void update_order(glm::vec3 player_position);

private:
  std::vector< BubbleData > bubble_data;
};
