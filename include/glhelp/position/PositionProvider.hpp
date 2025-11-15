#pragma once

#include <concepts>
#include <optional>

#include <glm/ext.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

namespace glhelp {

constexpr inline static glm::vec3 UP_VECTOR{0, 1, 0};
constexpr inline static glm::vec3 RIGHT_VECTOR{1, 0, 0};
constexpr inline static glm::vec3 FORWARD_VECTOR{0, 0, 1};

template< typename T >
concept PositionProvider = requires(const T& a) {
  { a.get_position() } -> std::convertible_to< glm::vec3 >;
  { a.get_rotation() } -> std::convertible_to< glm::quat >;
  { a.get_scale() } -> std::convertible_to< glm::vec3 >;
};

template< typename T >
concept CachingPositionProvider = requires(const T& a) {
  requires PositionProvider< T >;
  { a.get_cached() } -> std::convertible_to< std::optional< glm::mat4 > >;
  { a.cache(glm::mat4{}) } -> std::convertible_to< glm::mat4 >;
};

template< typename T >
concept PositionController = requires(T& a, float f) {
  requires PositionProvider< T >;
  { a.look_right(f) };
  { a.look_up(f) };
  { a.roll_cc(f) };
  { a.move_forwards(f) };
  { a.strafe(f) };
  { a.move_up(f) };
};


template< PositionProvider PositionSource >
auto calculate_model_matrix(const PositionSource& position_source) -> glm::mat4
{
  const glm::mat4 rotation_mat{glm::mat4_cast(position_source.get_rotation())};
  // TODO: Explicitly define the following matrices.
  const glm::mat4 translation_mat{glm::translate(glm::mat4{1.0}, position_source.get_position())};
  const glm::mat4 scale_mat{glm::scale(glm::mat4{1.0}, position_source.get_scale())};

  return translation_mat * rotation_mat * scale_mat;
}

template< PositionProvider PositionSource >
  requires(!CachingPositionProvider< PositionSource >)
auto get_model_matrix(const PositionSource& position_source) -> glm::mat4
{
  return calculate_model_matrix(position_source);
}

template< CachingPositionProvider PositionSource >
auto get_model_matrix(const PositionSource& position_source) -> glm::mat4
{
  return position_source.get_cached().value_or(
      position_source.cache(calculate_model_matrix(position_source)));
}

} // namespace glhelp
