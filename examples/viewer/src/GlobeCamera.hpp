#pragma once

#include <glm/glm.hpp>

#include <glhelp/Camera.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/position/Position.hpp>

class GlobePosition {
public:
  GlobePosition(glm::vec2 position, float zoom, float min_radius, float movement_speed = 100.0F);

  [[nodiscard]] auto get_globe_scale() const noexcept -> float { return zoom * zoom; }

  void init_mouse(glhelp::Window& window);

  float movement_speed;
  float mouse_rotation_speed{1.0F};
  const float min_radius;

  void poll_keyboard_events(glhelp::Window& window, float frame_time);

  [[nodiscard]] auto get_position() const -> glm::vec3;
  [[nodiscard]] auto get_rotation() const -> glm::quat;
  [[nodiscard]] auto get_scale() const -> glm::vec3;

  [[nodiscard]] auto get_radius() const noexcept -> float { return min_radius + min_radius * zoom; }

  bool enabled{false};

  float MaxZoom{0.01};
  static constexpr float MinZoom{3.0F};

private:
  float zoom;

  bool lmb_pressed{false};
  bool rmb_pressed{false};
  glhelp::Window::MouseEventDelegate mouse_event;
  glhelp::Window::ScrollEventDelegate scroll_event;
  glhelp::Window::MouseButtonEventDelegate mouse_button_event;

  float lat, lon;
  mutable glm::vec3 cached_position;
  mutable glm::quat cached_rotation;
  glm::vec3 offset_rotation{};
};

class GlobeCamera : public glhelp::Camera< GlobePosition > {
public:
  GlobeCamera(std::shared_ptr< glhelp::Window > window, GlobePosition&& globe_position);
};
