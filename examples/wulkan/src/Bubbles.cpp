#include <glhelp/position/PositionProvider.hpp>

#include "Bubbles.hpp"

static auto closestPointTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) -> glm::vec3
{
  const glm::vec3 ab = b - a;
  const glm::vec3 ac = c - a;
  const glm::vec3 ap = p - a;

  const float d1 = dot(ab, ap);
  const float d2 = dot(ac, ap);
  if (d1 <= 0.f && d2 <= 0.f)
    return a; // #1

  const glm::vec3 bp = p - b;
  const float d3 = dot(ab, bp);
  const float d4 = dot(ac, bp);
  if (d3 >= 0.f && d4 <= d3)
    return b; // #2

  const glm::vec3 cp = p - c;
  const float d5 = dot(ab, cp);
  const float d6 = dot(ac, cp);
  if (d6 >= 0.f && d5 <= d6)
    return c; // #3

  const float vc = d1 * d4 - d3 * d2;
  if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f) {
    const float v = d1 / (d1 - d3);
    return a + v * ab; // #4
  }

  const float vb = d5 * d2 - d1 * d6;
  if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f) {
    const float v = d2 / (d2 - d6);
    return a + v * ac; // #5
  }

  const float va = d3 * d6 - d5 * d4;
  if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f) {
    const float v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    return b + v * (c - b); // #6
  }

  const float denom = 1.f / (va + vb + vc);
  const float v = vb * denom;
  const float w = vc * denom;
  return a + v * ab + w * ac; // #0
}

auto get_bubble_positions(unsigned n, glm::vec3 obstacle_scale, std::mt19937& dev) -> std::vector< glm::mat4 >
{
  std::uniform_real_distribution< float > rotation_dist{0.0F, glm::pi< float >() * 2.0F};

  std::vector< glm::mat4 > positions;
  positions.reserve(n * n * n);
  for (unsigned x{}; x < n; x++) {
    for (unsigned y{}; y < n; y++) {
      for (unsigned z{}; z < n; z++) {
        const glm::vec3 position{
            static_cast< float >(x) / static_cast< float >(n - 1),
            static_cast< float >(y) / static_cast< float >(n - 1),
            static_cast< float >(z) / static_cast< float >(n - 1),
        };
        auto position_transform{glm::translate(glm::mat4{1.0F}, position)};
        auto rotation_transform{glm::mat4_cast(glm::quat{glm::vec3{rotation_dist(dev), rotation_dist(dev), rotation_dist(dev)}})};
        auto scale_transform{glm::scale(glm::mat4{1.0F}, obstacle_scale)};
        positions.emplace_back(position_transform * rotation_transform * scale_transform);
      }
    }
  }

  positions.pop_back(); // Remove the last one - make space for the finish.
  return positions;
}

Boubbles::Boubbles(
  std::shared_ptr< glhelp::ShaderProgram > shader,
  glhelp::SimplePosition position,
  const obj_parser::Obj< obj_parser::VertexNormals >& sphere,
  std::vector< glm::mat4 > bubble_positions)
  : InstancedMesh3d< glhelp::SimplePosition, glm::mat4 >(
      position,
      std::move(shader),
      sphere,
      bubble_positions),
    bubble_positions(std::move(bubble_positions))
{
}
