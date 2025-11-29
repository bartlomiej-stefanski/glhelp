#include <algorithm>

#include <glhelp/position/PositionProvider.hpp>

#include "Bubbles.hpp"
#include "Plane.hpp"

// static auto closestPointTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) -> glm::vec3
// {
//   const glm::vec3 ab = b - a;
//   const glm::vec3 ac = c - a;
//   const glm::vec3 ap = p - a;

//   const float d1 = dot(ab, ap);
//   const float d2 = dot(ac, ap);
//   if (d1 <= 0.f && d2 <= 0.f)
//     return a; // #1

//   const glm::vec3 bp = p - b;
//   const float d3 = dot(ab, bp);
//   const float d4 = dot(ac, bp);
//   if (d3 >= 0.f && d4 <= d3)
//     return b; // #2

//   const glm::vec3 cp = p - c;
//   const float d5 = dot(ab, cp);
//   const float d6 = dot(ac, cp);
//   if (d6 >= 0.f && d5 <= d6)
//     return c; // #3

//   const float vc = d1 * d4 - d3 * d2;
//   if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f) {
//     const float v = d1 / (d1 - d3);
//     return a + v * ab; // #4
//   }

//   const float vb = d5 * d2 - d1 * d6;
//   if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f) {
//     const float v = d2 / (d2 - d6);
//     return a + v * ac; // #5
//   }

//   const float va = d3 * d6 - d5 * d4;
//   if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f) {
//     const float v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
//     return b + v * (c - b); // #6
//   }

//   const float denom = 1.f / (va + vb + vc);
//   const float v = vb * denom;
//   const float w = vc * denom;
//   return a + v * ab + w * ac; // #0
// }

auto get_bubble_info(unsigned n, std::mt19937& rng) -> std::vector< BubbleInfo >
{
  auto real_dist{std::uniform_real_distribution< float >(0.0F, 1.0F)};

  std::vector< BubbleInfo > info;
  info.reserve(n * n * n);
  for (unsigned x{}; x < n; x++) {
    for (unsigned z{}; z < n; z++) {
      BubbleInfo bubble_info{
          .hsl_color = 2 * glm::pi< float >() * real_dist(rng),
          .time_offset = real_dist(rng) * glm::pi< float >() * 2,
          ._fpadd1 = 0.0F,
          .max_heigth = 20.0F,
          .start_height = 0.0F,
          ._fpadd2 = 0.0F,
      };
      info.emplace_back(bubble_info);
    }
  }

  return info;
}

auto get_bubble_positions(unsigned n, std::mt19937& rng) -> std::vector< glm::vec3 >
{
  auto real_dist{std::uniform_real_distribution< float >(0.0F, 0.8F)};

  std::vector< glm::vec3 > positions;
  positions.reserve(n * n * n);
  for (unsigned x{}; x < n; x++) {
    for (unsigned z{}; z < n; z++) {
      const auto angle{2 * glm::pi< float >() * (static_cast< float >(z) + real_dist(rng)) / static_cast< float >(n)};
      const auto distance{600 * glm::pow((static_cast< float >(x) + real_dist(rng)) / static_cast< float >(n), 2.0F) + 20.0};

      const auto xpos{glm::cos(angle) * distance};
      const auto zpos{glm::sin(angle) * distance};
      const auto h{get_mountain_heigth(xpos, zpos)};

      const glm::vec3 position{xpos, h, zpos};
      positions.emplace_back(position);
    }
  }

  return positions;
}

auto get_bubble_data(unsigned n, std::mt19937& rng) -> std::vector< BubbleData >
{
  const auto positions{get_bubble_positions(n, rng)};
  const auto infos{get_bubble_info(n, rng)};

  std::vector< BubbleData > data;
  data.reserve(positions.size());
  for (std::size_t i{}; i < positions.size(); i++) {
    data.emplace_back(BubbleData{
        .data = {
            .position = positions[i],
            .info = infos[i],
        }});
  }

  return data;
}

static auto bubble_data_to_mat3(std::vector< BubbleData >& v) -> std::vector< BubbleData_mat3 >
{
  static_assert(sizeof(BubbleData) == sizeof(glm::mat3), "Bubble Data should have the size of mat3");
  return *reinterpret_cast< std::vector< BubbleData_mat3 >* >(&v);
}

Bubbles::Bubbles(
    std::shared_ptr< glhelp::ShaderProgram > shader,
    glhelp::SimplePosition position,
    const obj_parser::Obj< obj_parser::VertexNormals >& sphere,
    std::vector< BubbleData > bubble_data)
    : InstancedMesh3d< glhelp::SimplePosition, BubbleData_mat3 >(
          position,
          std::move(shader),
          sphere,
          std::tuple{bubble_data_to_mat3(bubble_data)}),
      bubble_data(std::move(bubble_data))
{
}

void Bubbles::update_order(glm::vec3 player_position)
{
  std::ranges::sort(bubble_data, [&](const BubbleData& d1, const BubbleData& d2) -> bool {
    return glm::distance(d1.data.position, player_position) < glm::distance(d2.data.position, player_position);
  });

  this->update_buffers({bubble_data_to_mat3(bubble_data)});
}
