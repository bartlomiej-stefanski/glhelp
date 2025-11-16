#pragma once

#include <functional>

#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/SimplePosition.hpp>

namespace glhelp {

/// Allows for following a PositionProvider object.
/// Implements PositionProvider interface.
template< PositionProvider PositionSource >
class PositionFollower {
public:
  PositionFollower(PositionSource& position_source)
      : position_source(position_source)
  {
  }

  PositionFollower(PositionSource& position_source, glm::vec3 offset, float offset_yaw, float offset_pitch, float offset_roll, glm::vec3 scale)
      : offset(offset), offset_rotation(glm::vec3(offset_pitch, offset_yaw, offset_roll)), scale(scale), position_source(position_source)
  {
  }

  [[nodiscard]] auto get_position() const -> glm::vec3
  {
    return position_source.get().get_position() + (position_source.get().get_rotation() * offset);
  }
  [[nodiscard]] auto get_rotation() const -> glm::quat
  {
    return offset_rotation;
  }

  // Scale has no sense for PositionFollower.
  [[nodiscard]] auto get_scale() const -> glm::vec3 { return scale; }

private:
  glm::vec3 offset{};
  glm::quat offset_rotation{};
  glm::vec3 scale{1.0, 1.0, 1.0};

  std::reference_wrapper< PositionSource > position_source;
};

} // namespace glhelp
