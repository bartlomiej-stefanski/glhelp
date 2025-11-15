#pragma once

#include <glhelp/position/PlayerController.hpp>
#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/utils/event.hpp>
#include <glhelp/window.hpp>

namespace glhelp {

template< PositionController Controller >
class InteractiveController : public Controller {
public:
  InteractiveController(Controller&& controller);

  void init_mouse(Window& window);
  void poll_keys(Window& window, float frame_time);

  Window::MouseEventDelegate mouse_event;

  float movement_speed{1.0F};
  float rotation_speed{1.0F};
  float mouse_rotation_speed{1.0F};
};

} // namespace glhelp

// Include template implemntation
#include <glhelp/position/InteractiveController_impl.hpp>
