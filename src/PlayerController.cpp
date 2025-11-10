#include <glm/ext.hpp>

#include <glhelp/position/PlayerController.hpp>
#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/SimplePosition.hpp>

namespace glhelp {

PlayerController::PlayerController(glm::vec3 position, float yaw, float pith, float roll)
    : SimplePosition(position, yaw, pith, roll)
{
}

void PlayerController::look_right(float yaw_angle)
{
  const glm::quat delta_yaw = glm::angleAxis(yaw_angle, UP_VECTOR);
  rotation = rotation * delta_yaw;
}

void PlayerController::look_up(float angle)
{
  const glm::quat delta_pitch = glm::angleAxis(angle, RIGHT_VECTOR);
  rotation = rotation * delta_pitch;
}

void PlayerController::roll_cc(float angle)
{
  const glm::quat delta_roll = glm::angleAxis(angle, FORWARD_VECTOR);
  rotation = rotation * delta_roll;
}

void PlayerController::move_forwards(float distance)
{
  position += (rotation * FORWARD_VECTOR) * distance;
}

void PlayerController::strafe(float distance)
{
  position += (rotation * RIGHT_VECTOR) * distance;
}

void PlayerController::move_up(float distance)
{
  position += (rotation * UP_VECTOR) * distance;
}

} // namespace glhelp
