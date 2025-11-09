#pragma once

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/window.hpp>

namespace glhelp {

template< GLuint... Buttons, typename Function >
bool call_button_function(Function fun, int button_state, Window& window)
{
  return fun((glfwGetKey(window.get_window(), Buttons) == button_state)...);
}

} // namespace glhelp
