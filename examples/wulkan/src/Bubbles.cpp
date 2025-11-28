#include <glhelp/position/PositionProvider.hpp>

#include "Bubbles.hpp"

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

auto get_bubble_info(unsigned n, std::mt19937& rng) -> std::vector< BubbleInfo_vec4 >
{
  auto real_dist{std::uniform_real_distribution< float >(0.0F, 1.0F)};

  std::vector< BubbleInfo_vec4 > info;
  info.reserve(n * n * n);
  for (unsigned x{}; x < n; x++) {
    for (unsigned y{}; y < n; y++) {
      for (unsigned z{}; z < n; z++) {
        BubbleInfo bubble_info{
            .hsl_color = 2 * glm::pi< float >() * real_dist(rng),
            .time = real_dist(rng) * glm::pi< float >(),
            .max_heigth = 5.0F,
            .start_height = 0.0F,
        };
        info.emplace_back(bubble_info.packed);
      }
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
    for (unsigned y{}; y < n; y++) {
      const glm::vec3 position{
          (static_cast< float >(x) + real_dist(rng)) * 14.0F,
          0.0F,
          (static_cast< float >(y) + real_dist(rng)) * 14.0F,
      };
      positions.emplace_back(position);
    }
  }

  return positions;
}

Bubbles::Bubbles(
    std::shared_ptr< glhelp::ShaderProgram > shader,
    glhelp::SimplePosition position,
    const obj_parser::Obj< obj_parser::VertexNormals >& sphere,
    std::vector< glm::vec3 > bubble_positions,
    std::vector< BubbleInfo_vec4 > bubble_info)
    : InstancedMesh3d< glhelp::SimplePosition, glm::vec3, BubbleInfo_vec4 >(
          position,
          std::move(shader),
          sphere,
          std::tuple{bubble_positions, bubble_info}),
      bubble_positions(std::move(bubble_positions))
{
  auto fake_bubble_info{reinterpret_cast< std::vector< BubbleInfo >* >(&bubble_info)};
  this->bubble_info = std::move(*fake_bubble_info);
}
