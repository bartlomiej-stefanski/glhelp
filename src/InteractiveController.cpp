#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include "glhelp/position/PlayerController.hpp"
#include <glhelp/position/InteractiveController.hpp>

namespace glhelp {

InteractiveController::InteractiveController(glm::vec3 position, float yaw, float pith, float roll)
    : PlayerController(position, yaw, pith, roll)
{
}

void InteractiveController::init_mouse(Window& window)
{
  mouse_event = window.mouse_event.connect([&](float xoffset, float yoffset) {
    look_up(yoffset * mouse_rotation_speed);
    look_right(-xoffset * mouse_rotation_speed);
  });
}

void InteractiveController::poll_keys(Window& window, float frame_time)
{
  if (glfwGetKey(window.get_window(), GLFW_KEY_W) == GLFW_PRESS) {
    move_forwards(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_S) == GLFW_PRESS) {
    move_forwards(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_D) == GLFW_PRESS) {
    strafe(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_A) == GLFW_PRESS) {
    strafe(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_SPACE) == GLFW_PRESS) {
    move_up(2.0f * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    move_up(-2.0F * frame_time);
  }

  if (glfwGetKey(window.get_window(), GLFW_KEY_UP) == GLFW_PRESS) {
    look_up(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_DOWN) == GLFW_PRESS) {
    look_up(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
    look_right(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
    look_right(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_E) == GLFW_PRESS) {
    roll_cc(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_Q) == GLFW_PRESS) {
    roll_cc(2.0F * frame_time);
  }
}

} // namespace glhelp
