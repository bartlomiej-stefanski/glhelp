#pragma once

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/Window.hpp>
#include <glhelp/utils/Event.hpp>

namespace glhelp {

template< GLuint... Buttons, typename Function >
auto call_button_function(Function fun, int button_state, Window& window) -> bool
{
  return fun((glfwGetKey(window.get_window(), Buttons) == button_state)...);
}

struct EventButton {
  EventButton(GLuint button, Window& window)
  {
    key_event_delegate = window.key_event[button].connect([&](int, int action, int, float) {
      if (action != GLFW_PRESS)
        return;
      if (state)
        return;
      state = true;
      this->event();
    });
  }

  EventButton(EventButton&) = delete;
  auto operator=(EventButton) -> EventButton& = delete;

  auto operator()() -> bool
  {
    if (state) {
      state = false;
      return true;
    }

    return false;
  }

  Event< void() > event;

private:
  std::shared_ptr< Event< void(int, int, int, float) >::EventDelegate > key_event_delegate;
  bool state{};
};

struct TogglingButton {
  TogglingButton(GLuint button, Window& window, bool default_state = false)
      : event_button(button, window), toggled(default_state)
  {
    key_event_delegate = event_button.event.connect([&]() {
      if (this->event_button()) {
        toggled ^= true;
      }
      this->event(toggled);
    });
  }

  TogglingButton(TogglingButton&) = delete;
  auto operator=(TogglingButton) -> TogglingButton& = delete;

  auto operator()() -> bool
  {
    return toggled;
  }

  Event< void(bool) > event;

private:
  std::shared_ptr< Event< void() >::EventDelegate > key_event_delegate;
  EventButton event_button;
  bool toggled{};
};

} // namespace glhelp
