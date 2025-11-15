#pragma once

#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/FPSSimplePosition.hpp>

namespace glhelp {

class FPSPlayerController : public FPSSimplePosition {
public:
  FPSPlayerController() = default;
  FPSPlayerController(FPSSimplePosition&& position, float move_speed = 1.0F, float rotate_speed = 1.0F);

  void look_right(float yaw_angle);
  void look_up(float pitch_angle);
  void roll_cc(float roll_angle);

  void move_forwards(float distance);
  void strafe(float distance);
  void move_up(float distance);

  float move_speed;
  float rotate_speed;
};

} // namespace glhelp
