#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "MapCamera.hpp"

MinimapPosition::MinimapPosition(glm::vec2 position, float z, float scale, float movement_speed)
    : glhelp::CachingSimplePosition({position, z}, 0.0F, 0.0F, 0.0F, glm::vec3{1.0F}), scale(scale), movement_speed(movement_speed)
{
}

void MinimapPosition::init_mouse(glhelp::Window& window)
{
  mouse_event = window.mouse_event.connect([this](float xoffset, float yoffset) {
    if (!enabled) {
      return;
    }

    if (mouse_pressed) {
      position.x -= xoffset * get_minimap_scale() * movement_speed * 0.7;
      position.y -= yoffset * get_minimap_scale() * movement_speed * 0.7;
    }
  });

  scroll_event = window.scroll_event.connect([this](float xoffset [[maybe_unused]], float yoffset) {
    if (!enabled) {
      return;
    }

    scale = std::max(0.05F, scale - yoffset * 0.1F);
  });

  mouse_button_event = window.mouse_button_event.connect([this](int button, int action) {
    if (!enabled) {
      return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      mouse_pressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      mouse_pressed = false;
    }
  });
}

void MinimapPosition::poll_keyboard_events(glhelp::Window& window, const float frame_time)
{
  if (glfwGetKey(window.get_window(), GLFW_KEY_UP) == GLFW_PRESS) {
    position.y += movement_speed * get_minimap_scale() * frame_time;
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_DOWN) == GLFW_PRESS) {
    position.y -= movement_speed * get_minimap_scale() * frame_time;
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
    position.x += movement_speed * get_minimap_scale() * frame_time;
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
    position.x -= movement_speed * get_minimap_scale() * frame_time;
  }
}

void MinimapPosition::set_lat_lon(glm::vec2 new_position)
{
  // Map into [-180, 180] x [-90, 90] region.
  new_position += glm::vec2{180.0F, 90.0F};
  new_position /= glm::vec2{360.0, 180.0};
  new_position.x = glm::fract(new_position.x);
  new_position.y = glm::fract(new_position.y);
  new_position *= glm::vec2{360.0, 180.0};
  new_position -= glm::vec2{180.0F, 90.0F};

  new_position.y = to_mercator(new_position.y);

  position.x = new_position.x;
  position.y = new_position.y;
}

MinimapCamera::MinimapCamera(std::shared_ptr< glhelp::Window >& window, glm::vec2 focus_point, float scale, float near_clip, float far_clip)
    : glhelp::Camera< MinimapPosition >(window, MinimapPosition(focus_point, 100.0F, scale), 90.0F, near_clip, far_clip)
{
  init_mouse(*window);
}

static auto floor_i16(float f) -> i16
{
  return static_cast< i16 >(std::floor(f));
}

static auto ceil_i16(float f) -> i16
{
  return static_cast< i16 >(std::floor(f));
}

auto MinimapCamera::get_terrain_map(const float scaling) const -> hgt::TerrainMapArea
{
  const glm::vec3 camera_pos{get_position()};
  const glm::vec3 real_pos{
      camera_pos.x, 2.0F * glm::degrees((std::atan(std::exp(glm::radians(camera_pos.y)))) - (glm::pi< float >() / 4.0F)), camera_pos.z};

  return hgt::TerrainMapArea{
      .lat_from = std::max(floor_i16(real_pos.y - get_minimap_scale()), static_cast< i16 >(-90)),
      .lat_to = std::min(ceil_i16(real_pos.y + get_minimap_scale()), static_cast< i16 >(90)),
      .lon_from = std::max(floor_i16(real_pos.x - get_minimap_scale()), static_cast< i16 >(-180)),
      .lon_to = std::min(ceil_i16(real_pos.x + get_minimap_scale()), static_cast< i16 >(180)),
      .layer = static_cast< u8 >(std::sqrt(get_minimap_scale()) * scaling)};
}
