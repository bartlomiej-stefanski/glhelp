#pragma once

#include <random>

#include <glm/glm.hpp>

#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/position/Position.hpp>

auto get_rhombus_positions(unsigned n, glm::vec3 obstacle_scale, std::mt19937& dev) -> std::vector< glm::mat4 >;

class Rombuses : public glhelp::InstancedMesh3d< glhelp::SimplePosition, glm::mat4 > {
public:
  Rombuses(std::shared_ptr< glhelp::ShaderProgram > shader, glhelp::SimplePosition position, std::vector< glm::mat4 > rombus_positions);

  [[nodiscard]] auto collides_with(glm::vec3 player_position, float tolerance) -> bool;

private:
  std::vector< glm::mat4 > rombus_positions;
};
