#pragma once

#include <glm/glm.hpp>

#include <glhelp/Camera.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/position/Position.hpp>

#include "Setup.hpp"

auto lat_lon_to_point(float latDeg, float lonDeg, float radius) -> glm::vec3;

class GlobePosition {
public:
  GlobePosition(glm::vec2 position, float zoom, float radius, float movement_speed = 100.0F);

  [[nodiscard]] auto get_globe_scale() const noexcept -> float { return zoom * zoom; }

  void init_mouse(glhelp::Window& window);

  float movement_speed;
  float mouse_rotation_speed{1.0F};

  void poll_keyboard_events(glhelp::Window& window, float frame_time);

  [[nodiscard]] auto get_position() const -> glm::vec3;
  [[nodiscard]] auto get_rotation() const -> glm::quat;
  [[nodiscard]] auto get_scale() const -> glm::vec3;

  [[nodiscard]] auto get_lat_lon() const noexcept -> glm::vec2 { return glm::vec2{lon, lat}; }
  void set_lat_lon(glm::vec2 position)
  {
    lat = position.y;
    lon = position.x;
  }

  [[nodiscard]] auto get_radius() const noexcept -> float { return Radius + Radius * zoom; }

  bool enabled{false};

  static constexpr float MaxZoom{MaxHeigth / Radius};
  static constexpr float MinZoom{3.0F};

private:
  const float radius;

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

  void update_clipping();
};
