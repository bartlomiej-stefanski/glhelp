#pragma once

#ifndef REC_INTERACTIVE_CONTROLLER_GUARD
#define REC_INTERACTIVE_CONTROLLER_GUARD

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/InteractiveController.hpp>

namespace glhelp {

template< PositionController Controller >
InteractiveController< Controller >::InteractiveController(Controller&& controller)
    : Controller(controller)
{
}

template< PositionController Controller >
void InteractiveController< Controller >::init_mouse(Window& window)
{
  mouse_event = window.mouse_event.connect([&](float xoffset, float yoffset) {
    this->look_up(yoffset * mouse_rotation_speed);
    this->look_right(-xoffset * mouse_rotation_speed);
  });
}

template< PositionController Controller >
void InteractiveController< Controller >::poll_keys(Window& window, float frame_time)
{
  if (glfwGetKey(window.get_window(), GLFW_KEY_W) == GLFW_PRESS) {
    this->move_forwards(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_S) == GLFW_PRESS) {
    this->move_forwards(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_D) == GLFW_PRESS) {
    this->strafe(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_A) == GLFW_PRESS) {
    this->strafe(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_SPACE) == GLFW_PRESS) {
    this->move_up(2.0f * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    this->move_up(-2.0F * frame_time);
  }

  if (glfwGetKey(window.get_window(), GLFW_KEY_UP) == GLFW_PRESS) {
    this->look_up(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_DOWN) == GLFW_PRESS) {
    this->look_up(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
    this->look_right(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
    this->look_right(2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_E) == GLFW_PRESS) {
    this->roll_cc(-2.0F * frame_time);
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_Q) == GLFW_PRESS) {
    this->roll_cc(2.0F * frame_time);
  }
}

} // namespace glhelp

#endif
