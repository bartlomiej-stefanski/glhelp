#pragma once

#include <functional>

#include <glhelp/primitives/PositionProvider.hpp>

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
      : position_source(position_source), offset(offset), offset_yaw(offset_yaw), offset_pitch(offset_pitch), offset_roll(offset_roll)
  {
  }

  glm::vec3 get_position() { return position_source->get_position() + offset; }
  float get_yaw() { return position_source->get_yaw() + offset_yaw; }
  float get_pitch() { return position_source->get_pitch() + offset_pitch; }
  float get_roll() { return position_source->get_roll() + offset_roll; }

  glm::vec3 offset{};
  float offset_yaw{}, offset_pitch{}, offset_roll{};

  std::reference_wrapper< PositionSource > position_source;
};

} // namespace glhelp
