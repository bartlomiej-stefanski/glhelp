#pragma once

#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/SimplePosition.hpp>

namespace glhelp {

class PlayerController : public SimplePosition {
public:
  PlayerController() = default;
  PlayerController(glm::vec3 position, float yaw, float pith, float roll);

  void look_right(float yaw_angle);
  void look_up(float pitch_angle);
  void roll_cc(float roll_angle);

  void move_forwards(float distance);
  void strafe(float distance);
  void move_up(float distance);
};

} // namespace glhelp
