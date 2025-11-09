#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <glhelp/shader.hpp>

namespace glhelp {

class Window {
public:
  Window(int width, int height, const std::string& name);

  Window(Window&& other) noexcept;
  Window& operator=(Window&& other) noexcept;
  Window(Window&) = delete;
  Window& operator=(Window&) = delete;

  ~Window();

  /// Get current window aspect ration (width / height).
  float aspect_ratio() const noexcept;

  /// Set of callbacks to be called when the window is resized.
  ///
  /// Note: This does not accept lambdas as they are not copyable.
  ///
  /// The arguments passed to the callback are (in order):
  /// - width: The new width of the window.
  /// - height: The new height of the window.
  std::unordered_set< void (*)(int, int) > resize_callbacks;

  /// Set of callbacks to be called when a given key is pressed or released.
  /// For non-async use cases polling is preferred.
  ///
  /// Note: This does not accept lambdas as they are not copyable.
  ///
  /// The arguments passed to the callback are (in order):
  /// - key: The key that was pressed or released.
  /// - scancode: The system-specific scancode of the key.
  /// - action: The action that was performed on the key (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
  /// - mods: Bitfield describing which modifier keys were held down.
  std::unordered_map< int, std::unordered_set< void (*)(int, int, int, int) > > key_callbacks;

  /// Runs the main loop synchronously.
  void run_synchronously(std::function< void(Window&, double) > main_loop);

  /// Returns the underlying GLFW window handle.
  GLFWwindow* get_window() noexcept { return window; }

private:
  /// Window dimensions in pixels.
  int width, height;

  GLFWwindow* window = nullptr;
  GLFWmonitor* monitor = nullptr;

  void resize_cb(int width, int height);
  void key_cb(int key, int scancode, int action, int mods);

  static void resize_callback(GLFWwindow* window, int new_width, int new_height);
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

} // namespace glhelp
