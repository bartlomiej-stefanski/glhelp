#include <algorithm>
#include <cstdlib>

#include <glhelp/position/PositionProvider.hpp>

#include "Bubbles.hpp"
#include "HealthBar.hpp"
#include "Mountain.hpp"

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
          .exploded_time = 0.0F,
          .max_heigth = 20.0F,
          .start_height = 0.0F,
          .size_multiplier = 1.0F,
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

void Bubbles::update_bubbles(glm::vec3 player_position, const float time, const float frame_time, bool should_sort, HealthBar& health_bar)
{
  // DEBUG: Add option to disable bubble sorting.
  if (should_sort) [[likely]] {
    std::ranges::sort(bubble_data, [&](const BubbleData& d1, const BubbleData& d2) -> bool {
      return glm::distance(d1.data.position, player_position) < glm::distance(d2.data.position, player_position);
    });
  }

  glm::vec3 turbulence_pointer{glm::sin(time / 3.0F), 0.0F, glm::cos(time / 3.0F) * 10.0F};

  for (auto& data : bubble_data) {
    const auto max_heigth{data.data.info.max_heigth};
    const auto start_heigth{data.data.info.start_height};
    const auto vertical_transition{(max_heigth - start_heigth) / 2};

    const auto bubble_time{time / 3.0F + data.data.info.time_offset + glm::pi< float >()};
    const auto cycle_start{2 * glm::pi< float >() * glm::floor(bubble_time / (2 * glm::pi< float >()))};
    // Value in range [0,1]. Determines the cycle position of the bubble.
    // Values in range [0, 0.5] mean a rising bubble and [0.5, 1.0) represents falling bubble.
    const auto cycle_percentage{(bubble_time - cycle_start) / (2 * glm::pi< float >())};

    data.data.info.size_multiplier = cycle_percentage * 5.0F + 1.0F;
    data.data.position += turbulence_pointer * frame_time;

    const float vertical_offset{10 * (glm::cos(bubble_time + glm::pi< float >()) + 1.0F + start_heigth) * vertical_transition};
    const glm::vec3 real_bubble_position{data.data.position + glhelp::UP_VECTOR * vertical_offset};
    const float distance{glm::distance(player_position, real_bubble_position)};

    // Pop boubles when they are close to the drone AND are not already popped.
    if (distance < data.data.info.size_multiplier + 2.0F && data.data.info.exploded_time > bubble_time) [[unlikely]] {
      std::cerr << "Player popped a bubble!\n";
      data.data.info.exploded_time = bubble_time;
      health_bar.record_collision();
    }

    // Close to recently-popped bubble.
    if (distance < data.data.info.size_multiplier * 4.0F && cycle_percentage > 0.4F && cycle_percentage < 0.8F) [[unlikely]] {
      health_bar.gas_damage(frame_time, distance);
    }

    if (data.data.info.exploded_time < cycle_start) [[unlikely]] {
      data.data.info.exploded_time = cycle_start + 0.8F * glm::pi< float >();
    }
  }

  this->update_buffers({bubble_data_to_mat3(bubble_data)});
}
