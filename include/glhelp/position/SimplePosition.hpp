#pragma once

#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

class SimplePosition {
public:
  SimplePosition() = default;

  SimplePosition(glm::vec3 position, float yaw, float pitch, float roll, glm::vec3 scale = glm::vec3{1.0F, 1.0F, 1.0F})
      : position(position), rotation(glm::vec3(pitch, yaw, roll)), scale(scale)
  {
  }

  [[nodiscard]] auto get_position() const -> glm::vec3 { return position; }
  [[nodiscard]] auto get_rotation() const -> glm::quat { return rotation; }
  [[nodiscard]] auto get_scale() const -> glm::vec3 { return scale; }

  void set_position(const glm::vec3& new_position)
  {
    position = new_position;
  }

  void set_rotation(float yaw, float pitch, float roll)
  {
    rotation = glm::quat(glm::vec3(pitch, yaw, roll));
  }

  void set_scale(const glm::vec3& new_scale)
  {
    scale = new_scale;
  }

protected:
  glm::vec3 position{};
  glm::quat rotation{};
  glm::vec3 scale{1.0, 1.0, 1.0};
};

} // namespace glhelp
