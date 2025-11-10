#pragma once

#include <GLFW/glfw3.h>

namespace glhelp {

class GLFWContext {
public:
  GLFWContext();
  ~GLFWContext();

  GLFWContext(const GLFWContext&) = delete;
  auto operator=(const GLFWContext&) -> GLFWContext& = delete;

  GLFWContext(GLFWContext&& other) noexcept;
  auto operator=(GLFWContext&& other) noexcept -> GLFWContext&;

  static auto is_initialized() -> bool { return initialized; }

private:
  inline static bool initialized{false};
  bool holds_context{false};
};

} // namespace glhelp
