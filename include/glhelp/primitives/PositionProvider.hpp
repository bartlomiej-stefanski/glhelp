#pragma once

#include <concepts>

#include <glm/glm.hpp>

namespace glhelp {

template< typename T >
concept PositionProvider = requires(T a) {
  { a.get_position() } -> std::convertible_to< glm::vec3 >;
  { a.get_yaw() } -> std::convertible_to< float >;
  { a.get_pitch() } -> std::convertible_to< float >;
  { a.get_roll() } -> std::convertible_to< float >;
};

constexpr inline static glm::vec3 UP_VECTOR{0, 1, 0};
constexpr inline static glm::vec3 RIGHT_VECTOR{1, 0, 0};
constexpr inline static glm::vec3 FORWARD_VECTOR{0, 0, 1};

} // namespace glhelp
