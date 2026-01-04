#pragma once

#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

#include <glhelp/Camera.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/Event.hpp>

#include "hgt/Hgt.hpp"

inline auto to_mercator(float degrees) -> float
{
  return glm::degrees(std::log(std::tan(
      (glm::pi< float >() / 4.0F) + glm::radians(degrees / 2.0F))));
}

inline auto from_mercator(float degrees) -> float
{
  return glm::degrees(std::atan(std::exp(glm::radians(degrees))) - (glm::pi< float >() / 4.0F)) * 2.0F;
  // return glm::degrees(glm::atan(std::exp(glm::radians(degrees))) - (glm::pi< float >() / 4.0F)) * 2.0F;
}

class MinimapPosition : public glhelp::CachingSimplePosition {
public:
  MinimapPosition(glm::vec2 position, float z, float scale, float movement_speed = 4.0F);

  void init_mouse(glhelp::Window& window);

  [[nodiscard]] auto get_minimap_scale() const noexcept -> float { return scale * scale; }

  void poll_keyboard_events(glhelp::Window& window, float frame_time);

  [[nodiscard]] auto get_lat_lon() const noexcept -> glm::vec2 { return glm::vec2{position.x, from_mercator(position.y)}; }
  void set_lat_lon(glm::vec2 position);

  bool enabled{true};

private:
  float scale;
  float movement_speed;

  glhelp::Window::MouseEventDelegate mouse_event;
  glhelp::Window::ScrollEventDelegate scroll_event;
  glhelp::Window::MouseButtonEventDelegate mouse_button_event;
  bool mouse_pressed{false};
};

class MinimapCamera : public glhelp::Camera< MinimapPosition > {
public:
  MinimapCamera(std::shared_ptr< glhelp::Window >& window, glm::vec2 focus_point, float scale, float near_clip, float far_clip);

  auto get_terrain_map(const float scaling = 1.0F) const -> hgt::TerrainMapArea;
};
