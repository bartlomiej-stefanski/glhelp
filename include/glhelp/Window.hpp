#pragma once

#include <functional>
#include <string>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/Shader.hpp>
#include <glhelp/utils/Event.hpp>

namespace glhelp {

class Window {
public:
  Window(int width, int height, const std::string& name, bool write_fps = false, GLuint cursor_mode = GLFW_CURSOR_DISABLED);

  Window(Window&) = delete;
  auto operator=(Window&) -> Window& = delete;
  Window(Window&& other) noexcept = delete;
  auto operator=(Window&& other) noexcept -> Window& = delete;

  ~Window();

  /// Get current window aspect ration (width / height).
  [[nodiscard]] auto aspect_ratio() const noexcept -> float;
  [[nodiscard]] auto get_size() const noexcept -> glm::vec2;

  /// Event that occurs each time a resize occurs.
  /// The arguments passed to the callback are (in order):
  /// - width: The new width of the window.
  /// - height: The new height of the window.
  Event< void(int, int) > resize_event;
  using ResizeEventDelegate = std::shared_ptr< decltype(resize_event)::EventDelegate >;

  /// Set of events to be generated when a given key is pressed or released.
  /// For non-async use cases polling is preferred.
  /// The arguments passed to the callback are (in order):
  /// - key: What key caused an event.
  /// - action: The action that was performed on the key (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
  /// - mods: Bitfield describing which modifier keys were held down.
  /// - frame_time: time since last frame.
  std::unordered_map< int, Event< void(int, int, int, float) > > key_event;
  using KeyEventDelegate = std::shared_ptr< decltype(key_event.begin()->second)::EventDelegate >;

  /// Event that occurs each time mouse is moved.
  /// The arguments passed to the callback are (in order):
  /// - xoffset: normalized (to screen size) movement in horizonstal direction.
  /// - yoffset: normalized (to screen size) movement in vertical direction.
  Event< void(float, float) > mouse_event;
  using MouseEventDelegate = std::shared_ptr< decltype(mouse_event)::EventDelegate >;

  /// Event that occurs each time mouse wheel is scrolled.
  /// The arguments passed to the callback are (in order):
  /// - xoffset: scroll in horizontal direction.
  /// - yoffset: scroll in vertical direction.
  Event< void(float, float) > scroll_event;
  using ScrollEventDelegate = std::shared_ptr< decltype(scroll_event)::EventDelegate >;

  /// Event that occurs each time mouse button is pressed/released.
  /// The arguments passed to the callback are (in order):
  /// - button: GLFW_MOUSE_BUTTON_* constant.
  /// - action: GLFW_PRESS or GLFW_RELEASE.
  Event< void(int, int) > mouse_button_event;
  using MouseButtonEventDelegate = std::shared_ptr< decltype(mouse_button_event)::EventDelegate >;

  /// Runs the main loop synchronously.
  void run_synchronously(const std::function< bool(Window&, double, double) >& main_loop);

  /// Returns the underlying GLFW window handle.
  [[nodiscard]] auto get_window() noexcept -> GLFWwindow* { return window; }

  [[nodiscard]] auto is_active() const -> bool { return !glfwWindowShouldClose(window); }

  bool write_fps{};
  /// Function will be called on each FPS print.
  /// It should return text to be displayed *without* any newlines.
  std::optional< std::function< std::string() > > fps_callback;
  void debug_print_fps(float frame_time);

private:
  /// Window dimensions in pixels.
  int width, height;
  float last_frame_time;

  GLuint primitiveQuery;
  bool query_started{false};
  GLuint triangles_drawn;

  GLFWwindow* window = nullptr;
  GLFWmonitor* monitor = nullptr;

  void resize_cb(int width, int height);
  void key_cb(int key, int scancode, int action, int mods);
  void mouse_cb(double xpos, double ypos);
  void scroll_cb(double xoffset, double yoffset);
  void mouse_button_cb(int button, int action, int mods);
  float last_xpos{}, last_ypos{};

  static void resize_callback(GLFWwindow* window, int new_width, int new_height);
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
  static void initial_mouse_callback(GLFWwindow* window, double xpos, double ypos);
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  static void initial_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
};

} // namespace glhelp
