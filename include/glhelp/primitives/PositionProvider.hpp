#pragma once

#include <concepts>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace glhelp {

template< typename T >
concept PositionProvider = requires(const T& a) {
  { a.get_position() } -> std::convertible_to< glm::vec3 >;
  { a.get_rotation() } -> std::convertible_to< glm::quat >;
};

constexpr inline static glm::vec3 UP_VECTOR{0, 1, 0};
constexpr inline static glm::vec3 RIGHT_VECTOR{1, 0, 0};
constexpr inline static glm::vec3 FORWARD_VECTOR{0, 0, 1};

/// Calculates the Model Matrix for a given PositionProvider
template< PositionProvider PositionSource >
inline glm::mat4 get_model_matrix(const PositionSource& position_source, glm::vec3 scale = glm::vec3{1.0f})
{
  const glm::mat4 rotation_mat{glm::mat4_cast(position_source.get_rotation())};
  // TODO: Explicitly define the following matrices.
  const glm::mat4 translation_mat{glm::translate(glm::mat4{1.0}, position_source.get_position())};
  const glm::mat4 scale_mat{glm::scale(glm::mat4{1.0}, scale)};

  return translation_mat * rotation_mat * scale_mat;
}

} // namespace glhelp
