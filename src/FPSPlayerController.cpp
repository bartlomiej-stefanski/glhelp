#include <glm/ext.hpp>

#include <glhelp/position/FPSPlayerController.hpp>
#include <glhelp/position/FPSSimplePosition.hpp>
#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

FPSPlayerController::FPSPlayerController(FPSSimplePosition&& position, float move_speed, float rotate_speed)
    : FPSSimplePosition(position), move_speed(move_speed), rotate_speed(rotate_speed)
{
}

void FPSPlayerController::look_right(float angle)
{
  rotation.y += angle * rotate_speed;
}

void FPSPlayerController::look_up(float angle)
{
  rotation.x = glm::clamp(rotation.x +  angle * rotate_speed, glm::radians(-89.0F), glm::radians(89.0F));
}

void FPSPlayerController::roll_cc(float angle)
{
  rotation.z += angle * rotate_speed;
}

void FPSPlayerController::move_forwards(float distance)
{
  position += (glm::quat(rotation) * FORWARD_VECTOR) * distance * move_speed;
}

void FPSPlayerController::strafe(float distance)
{
  position += (glm::quat(rotation) * RIGHT_VECTOR) * distance * move_speed;
}

void FPSPlayerController::move_up(float distance)
{
  position += (glm::quat(rotation) * UP_VECTOR) * distance * move_speed;
}

} // namespace glhelp
