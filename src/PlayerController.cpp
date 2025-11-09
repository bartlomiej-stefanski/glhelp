#include <glm/ext.hpp>

#include <glhelp/primitives/PlayerController.hpp>
#include <glhelp/primitives/PositionProvider.hpp>

namespace glhelp {

PlayerController::PlayerController(glm::vec3 position, float yaw, float pith, float roll)
    : position(position), yaw(yaw), pitch(pith), roll(roll)
{
}

void PlayerController::look_right(float angle)
{
  yaw += glm::cos(roll) * angle;
  pitch += glm::sin(roll) * angle;
}

void PlayerController::look_up(float angle)
{
  pitch += glm::cos(roll) * angle;
  yaw += glm::sin(roll) * angle;
}

void PlayerController::roll_cc(float angle)
{
  roll += angle;
}

void PlayerController::move_forwards(float distance)
{
  const glm::quat rotation{glm::angleAxis(yaw, UP_VECTOR) * glm::angleAxis(pitch, RIGHT_VECTOR)};
  position += FORWARD_VECTOR * rotation * distance;
}

void PlayerController::strafe(float distance)
{
  const glm::quat rotation{glm::angleAxis(yaw, UP_VECTOR) * glm::angleAxis(roll, FORWARD_VECTOR)};
  position += RIGHT_VECTOR * rotation * distance;
}

void PlayerController::move_up(float distance)
{
  const glm::quat rotation{glm::angleAxis(pitch, RIGHT_VECTOR) * glm::angleAxis(roll, FORWARD_VECTOR)};
  position += UP_VECTOR * rotation * distance;
}

void PlayerController::set_rotation(float new_yaw, float new_pitch, float new_roll)
{
  yaw = new_yaw;
  pitch = new_pitch;
  roll = new_roll;
}

} // namespace glhelp
