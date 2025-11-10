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
  PositionFollower(const PositionSource& position_source)
      : position_source(position_source)
  {
  }

  PositionFollower(const PositionSource& position_source, glm::vec3 offset, float offset_yaw, float offset_pitch, float offset_roll)
      : position_source(position_source), offset(offset), offset_rotation(glm::vec3(offset_pitch, offset_yaw, offset_roll))
  {
  }

  [[nodiscard]] auto get_position() const -> glm::vec3
  {
    return position_source->get_position() + (position_source->get_rotation() * offset);
  }
  [[nodiscard]] auto get_rotation() const -> glm::vec3
  {
    return position_source->get_rotation() * offset_rotation;
  }

  // Scale has no sense for PositionFollower.
  [[nodiscard]] auto get_scale() const -> glm::vec3 { return glm::vec3{1.0F}; }

private:
  glm::vec3 offset{};
  glm::quat offset_rotation{};

  std::reference_wrapper< PositionSource > position_source;
};

} // namespace glhelp
