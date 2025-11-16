#pragma once

#include <vector>

#include <glm/glm.hpp>

struct Sphere {
  std::vector< glm::vec3 > vertices;
  std::vector< unsigned > indices;
};

auto generate_sphere(unsigned sector_count, unsigned stack_count) -> Sphere;

inline Sphere player_sphere{generate_sphere(10, 10)};
