#include <glm/ext.hpp>

#include <glhelp/position/PlayerController.hpp>
#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/SimplePosition.hpp>

namespace glhelp {

PlayerController::PlayerController(SimplePosition&& position, float move_speed, float rotate_speed)
    : SimplePosition(position), move_speed(move_speed), rotate_speed(rotate_speed)
{
}

void PlayerController::look_right(float angle)
{
  const glm::quat delta_yaw = glm::angleAxis(angle, UP_VECTOR);
  rotation = rotation * delta_yaw * rotate_speed;
}

void PlayerController::look_up(float angle)
{
  const glm::quat delta_pitch = glm::angleAxis(angle, RIGHT_VECTOR);
  rotation = rotation * delta_pitch * rotate_speed;
}

void PlayerController::roll_cc(float angle)
{
  const glm::quat delta_roll = glm::angleAxis(angle, FORWARD_VECTOR);
  rotation = rotation * delta_roll * rotate_speed;
}

void PlayerController::move_forwards(float distance)
{
  position += (rotation * FORWARD_VECTOR) * distance * move_speed;
}

void PlayerController::strafe(float distance)
{
  position += (rotation * RIGHT_VECTOR) * distance * move_speed;
}

void PlayerController::move_up(float distance)
{
  position += (rotation * UP_VECTOR) * distance * move_speed;
}

} // namespace glhelp
