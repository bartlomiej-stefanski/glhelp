#include <obj_parser/Vertex.hpp>

#include "FastNoiseLite.hpp"
#include "Mountain.hpp"

auto get_mountain_heigth(float x, float z) -> float
{
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  noise.SetFractalType(FastNoiseLite::FractalType_FBm);
  noise.SetFractalOctaves(4);

  auto hyperboloid{[](float x) -> float {
    constexpr auto max_heigth{225.0F};
    const auto heigth{glm::min(glm::max(700.0F / ((x / 70.0F) + 3.0F), 15.0F), 225.0F)};
    if (heigth == max_heigth) {
      return heigth - 7.5f + x;
    }
    return heigth;
  }};

  const auto r{glm::sqrt(x * x + z * z)};
  const auto base_heigth{hyperboloid(r)};
  float noise_val{noise.GetNoise(x * 2.0f, z * 2.0f)};

  noise_val = 1.0f - glm::abs(noise_val);
  noise_val = noise_val * noise_val;

  // Gropws with the mountain height (untill we get to the 'snow' region
  const auto blend_factor{base_heigth < 200
                              ? glm::clamp((base_heigth - 60.0f) / 100.0f, 0.0f, 1.0f)
                              : 1.0f};

  const auto noise_strength{5.0F};
  const auto snow_offset{base_heigth > 200 ? noise_strength - 2.0F : 0.0F};

  const auto final_height{glm::max< float >(base_heigth + snow_offset, base_heigth + (noise_val * noise_strength * blend_factor))};
  return final_height;
}

auto get_mountain_normal(float x, float z) -> glm::vec3
{
  constexpr auto delta{0.001F};
  const auto h_L{get_mountain_heigth(x - delta, z)};
  const auto h_R{get_mountain_heigth(x + delta, z)};
  const auto h_D{get_mountain_heigth(x, z - delta)};
  const auto h_U{get_mountain_heigth(x, z + delta)};

  const glm::vec3 tangent_x{2.0F * delta, h_R - h_L, 0.0F};
  const glm::vec3 tangent_z{0.0F, h_U - h_D, 2.0F * delta};

  const glm::vec3 normal{glm::normalize(glm::cross(tangent_z, tangent_x))};
  return normal;
}

auto get_mountain_vertices(unsigned n) -> std::vector< obj_parser::VertexNormals >
{
  std::vector< obj_parser::VertexNormals > vertices;
  vertices.reserve(n * n);

  for (unsigned x = 0; x < n; x++) {
    for (unsigned z = 0; z < n; z++) {
      const auto angle{2 * glm::pi< float >() * static_cast< float >(z) / static_cast< float >(n)};
      const auto distance{1000 * glm::pow(static_cast< float >(x) / static_cast< float >(n), 2.0F) + 2.0};

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

auto Mountain::check_collision(glm::vec3 position, float radius) const -> bool
{
  if (position.y - radius <= get_mountain_heigth(position.x, position.z)) {
    return true;
  }
  else {
    return false;
  }
}
