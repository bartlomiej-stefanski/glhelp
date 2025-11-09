#pragma once

#include <glhelp/primitives/PositionProvider.hpp>

namespace glhelp {

class PlayerController {
public:
  PlayerController() = default;
  PlayerController(glm::vec3 position, float yaw, float pith, float roll);

  void look_right(float angle);
  void look_up(float angle);
  void roll_cc(float angle);

  void move_forwards(float distance);
  void strafe(float distance);
  void move_up(float distance);

  void set_position(const glm::vec3& new_position) { position = new_position; }
  void set_rotation(float new_yaw, float new_pitch, float new_roll);

  const glm::vec3& get_position() const { return position; }
  float get_yaw() { return yaw; }
  float get_pitch() { return pitch; }
  float get_roll() { return roll; }

  glm::vec3 position{};
  float yaw{}, pitch{}, roll{};
};

} // namespace glhelp
