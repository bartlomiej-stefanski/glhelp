#pragma once

#include <GLFW/glfw3.h>

namespace glhelp {

class GLFWContext {
public:
  GLFWContext();
  ~GLFWContext();

  GLFWContext(const GLFWContext&) = delete;
  GLFWContext& operator=(const GLFWContext&) = delete;

  GLFWContext(GLFWContext&& other) noexcept;
  GLFWContext& operator=(GLFWContext&& other) noexcept;

  static bool is_initialized() { return initialized; }

private:
  inline static bool initialized{false};
  bool holds_context{false};
};

} // namespace glhelp
