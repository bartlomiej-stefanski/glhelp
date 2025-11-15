#include <stdexcept>

#include <GLFW/glfw3.h>

#include <glhelp/utils/GLFWContext.hpp>

namespace glhelp {

GLFWContext::GLFWContext() : holds_context(true)
{
  if (initialized) {
    throw std::runtime_error("GLFW context already initialized!");
  }

  if (!glfwInit()) {
    throw std::runtime_error("Error initializing GLFW!");
  }

  initialized = true;
}

GLFWContext::~GLFWContext()
{
  if (holds_context) {
    initialized = false;
    glfwTerminate();
  }
}

GLFWContext::GLFWContext(GLFWContext&& other) noexcept : holds_context(other.holds_context)
{
  other.holds_context = false;
}

auto GLFWContext::operator=(GLFWContext&& other) noexcept -> GLFWContext&
{
  if (holds_context) {
    initialized = false;
    glfwTerminate();
  }

  holds_context = other.holds_context;
  other.holds_context = false;
  return *this;
}

} // namespace glhelp
