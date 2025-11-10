#pragma once

#include <functional>
#include <string>

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <glhelp/shader.hpp>
#include <glhelp/utils/event.hpp>

namespace glhelp {

class Window {
public:
  Window(int width, int height, const std::string& name);

  Window(Window&& other) noexcept;
  auto operator=(Window&& other) noexcept -> Window&;
  Window(Window&) = delete;
  auto operator=(Window&) -> Window& = delete;

  ~Window();

  /// Get current window aspect ration (width / height).
  auto aspect_ratio() const noexcept -> float;

  /// Event that occurs each time a resize occurs.
  /// The arguments passed to the callback are (in order):
  /// - width: The new width of the window.
  /// - height: The new height of the window.
  Event< void(int, int) > resize_event;

  /// Set of events to be generated when a given key is pressed or released.
  /// For non-async use cases polling is preferred.
  /// The arguments passed to the callback are (in order):
  /// - key: The key that was pressed or released.
  /// - scancode: The system-specific scancode of the key.
  /// - action: The action that was performed on the key (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
  /// - mods: Bitfield describing which modifier keys were held down.
  std::unordered_map< int, Event< void(int, int, int, int) > > key_event;

  /// Event that occurs each time mouse is moved.
  /// The arguments passed to the callback are (in order):
  /// - xoffset: normalized (to screen size) movement in horizonstal direction.
  /// - yoffset: normalized (to screen size) movement in vertical direction.
  Event< void(float, float) > mouse_event;

  /// Runs the main loop synchronously.
  void run_synchronously(const std::function< void(Window&, double, double) >& main_loop);

  /// Returns the underlying GLFW window handle.
  auto get_window() noexcept -> GLFWwindow* { return window; }

private:
  /// Window dimensions in pixels.
  int width, height;

  GLFWwindow* window = nullptr;
  GLFWmonitor* monitor = nullptr;

  void resize_cb(int width, int height);
  void key_cb(int key, int scancode, int action, int mods);
  void mouse_cb(double xpos, double ypos);
  float last_xpos{}, last_ypos{};

  static void resize_callback(GLFWwindow* window, int new_width, int new_height);
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
  static void initial_mouse_callback(GLFWwindow* window, double xpos, double ypos);
};

} // namespace glhelp
