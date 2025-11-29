#include <obj_parser/Vertex.hpp>

#include "Plane.hpp"

auto get_mountain_heigth(float x, float z) -> float
{
  auto hyperboloid{[](float x) -> float {
    return glm::max(700.0F / ((x / 70.0F) + 3.0F), 5.0F);
  }};

  const auto r{glm::sqrt(x * x + z * z)};
  return hyperboloid(r);
}

auto get_mountain_normal(float x, float z) -> glm::vec3
{
  constexpr auto delta{0.001F};
  const auto below_x{get_mountain_heigth(x - delta, z)};
  const auto above_x{get_mountain_heigth(x + delta, z)};
  const auto below_z{get_mountain_heigth(x, z - delta)};
  const auto above_z{get_mountain_heigth(x, z + delta)};

  const auto tangent_x{glm::normalize(glm::vec3{above_x - below_x, delta, 0.0F})};
  const auto tangent_z{glm::normalize(glm::vec3{0.0F, delta, above_z - below_z})};

  auto result{glm::normalize(tangent_z + tangent_x)};

  if (glm::dot(result, glm::vec3(0, 1, 0)) < 0) {
    result *= -1;
  }

  return result;
}

auto get_mountain_vertices(unsigned n) -> std::vector< obj_parser::VertexNormals >
{
  std::vector< obj_parser::VertexNormals > vertices;
  vertices.reserve(n * n);

  for (unsigned x = 0; x < n; x++) {
    for (unsigned z = 0; z < n; z++) {
      const auto angle{2 * glm::pi< float >() * static_cast< float >(z) / static_cast< float >(n)};
      const auto distance{700 * glm::pow(static_cast< float >(x) / static_cast< float >(n), 2.0F) + 5.0};

      // Center the plane around (0, 0, 0)
      const auto xpos{glm::cos(angle) * distance};
      const auto zpos{glm::sin(angle) * distance};
      const auto h{get_mountain_heigth(xpos, zpos)};
      vertices.emplace_back(glm::vec3{xpos, h, zpos}, get_mountain_normal(xpos, zpos));
    }
  }

  return vertices;
}

auto get_mountain_indices(unsigned n) -> std::vector< unsigned >
{
  std::vector< unsigned > indices;
  indices.reserve(n * n * 3 * 2);

  for (unsigned x = 0; x < n - 1; x++) {
    for (unsigned z = 0; z < n - 1; z++) {
      const auto top_left{x * n + z};
      const auto top_right{top_left + 1};
      const auto bottom_left{(x + 1) * n + z};
      const auto bottom_right{bottom_left + 1};

      // First triangle
      indices.emplace_back(bottom_left);
      indices.emplace_back(top_left);
      indices.emplace_back(top_right);

      // Second triangle
      indices.emplace_back(bottom_left);
      indices.emplace_back(top_right);
      indices.emplace_back(bottom_right);
    }

    const auto top_first{x * n};
    const auto top_last{top_first + (n - 1)};
    const auto bottom_first{(x + 1) * n};
    const auto bottom_last{bottom_first + (n - 1)};

    indices.emplace_back(top_first);
    indices.emplace_back(bottom_first);
    indices.emplace_back(bottom_last);

    indices.emplace_back(top_first);
    indices.emplace_back(bottom_last);
    indices.emplace_back(top_last);
  }

  return indices;
}
