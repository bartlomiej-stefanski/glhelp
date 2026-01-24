#include <algorithm>

#include <glm/trigonometric.hpp>

#include <glhelp/Window.hpp>
#include <glhelp/position/Position.hpp>

#include "GlobeCamera.hpp"

auto lat_lon_to_point(float latDeg, float lonDeg, float radius) -> glm::vec3
{
  const float latRad = glm::radians(latDeg);
  const float lonRad = glm::radians(lonDeg);

  const float y = radius * std::sin(latRad);
  const float rCosLat = radius * std::cos(latRad);
  const float x = rCosLat * std::sin(lonRad);
  const float z = rCosLat * std::cos(lonRad);

  return glm::vec3{x, y, z};
}

GlobePosition::GlobePosition(glm::vec2 position, float zoom, float min_radius, float movement_speed)
    : movement_speed(movement_speed), min_radius(min_radius * 0.5), zoom(zoom), lat(position.y), lon(position.x)
{
}

void GlobePosition::init_mouse(glhelp::Window& window)
{
  mouse_event = window.mouse_event.connect([this](float xoffset, float yoffset) {
    const auto off_rot_cos{glm::cos(offset_rotation.z)};
    const auto off_rot_sin{glm::sin(offset_rotation.z)};

    if (!enabled) {
      return;
    }

    if (lmb_pressed) {
      lon += (off_rot_cos * xoffset - off_rot_sin * yoffset) * movement_speed;
      lat = glm::clamp(lat + (off_rot_sin * xoffset + off_rot_cos * yoffset) * movement_speed, -89.0F, 89.0F);
    }
  });

  scroll_event = window.scroll_event.connect([this](float xoffset [[maybe_unused]], float yoffset) {
    if (!enabled) {
      return;
    }

    zoom = glm::clamp(std::max(MaxZoom, zoom - yoffset * zoom * 0.25F), MaxZoom, MinZoom / 3.0F);
  });

  mouse_button_event = window.mouse_button_event.connect([this](int button, int action) {
    if (!enabled) {
      return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      lmb_pressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      lmb_pressed = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
      rmb_pressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
      rmb_pressed = false;
    }
  });
}

void GlobePosition::poll_keyboard_events(glhelp::Window& window, const float frame_time)
{
  const auto off_rot_cos{glm::cos(offset_rotation.z)};
  const auto off_rot_sin{glm::sin(offset_rotation.z)};

  float x_speed{};
  float y_speed{};

  if (glfwGetKey(window.get_window(), GLFW_KEY_W) == GLFW_PRESS) {
    y_speed += movement_speed * frame_time;
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_S) == GLFW_PRESS) {
    y_speed -= movement_speed * frame_time;
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_D) == GLFW_PRESS) {
    x_speed += movement_speed * frame_time;
  }
  if (glfwGetKey(window.get_window(), GLFW_KEY_A) == GLFW_PRESS) {
    x_speed -= movement_speed * frame_time;
  }

  lon += (off_rot_cos * x_speed - off_rot_sin * y_speed) * zoom;
  lat = glm::clamp(lat + (off_rot_sin * x_speed + off_rot_cos * y_speed) * zoom, -30.0F, 89.0F);
}

auto GlobePosition::get_position() const -> glm::vec3
{
  cached_position = lat_lon_to_point(lat, lon, get_radius());
  cached_rotation = glm::quatLookAt(glm::normalize(-cached_position), glhelp::UP_VECTOR) * glm::quat(offset_rotation);
  return cached_position;
}

auto GlobePosition::get_rotation() const -> glm::quat
{
  return cached_rotation;
}

auto GlobePosition::get_scale() const -> glm::vec3
{
  return glm::vec3{1.0F};
}

GlobeCamera::GlobeCamera(std::shared_ptr< glhelp::Window > window, GlobePosition&& globe_position)
    : glhelp::Camera< GlobePosition >(std::move(window), globe_position, 90.0F, 0.1F, 1000.0F)
{
}
