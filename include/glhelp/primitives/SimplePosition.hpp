#pragma once

#include <glhelp/primitives/PositionProvider.hpp>

namespace glhelp {

class SimplePosition {
public:
  SimplePosition() = default;

  SimplePosition(glm::vec3 position, float yaw, float pitch, float roll)
      : position(position), yaw(yaw), pitch(pitch), roll(roll)
  {
  }

  glm::vec3 get_position() { return position; }
  float get_yaw() { return yaw; }
  float get_pitch() { return pitch; }
  float get_roll() { return roll; }

  glm::vec3 position{};
  float yaw{}, pitch{}, roll{};
};

} // namespace glhelp
