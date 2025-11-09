#pragma once

#include <glhelp/primitives/PositionProvider.hpp>

namespace glhelp {

class SimplePosition {
public:
  SimplePosition() = default;

  SimplePosition(glm::vec3 position, float yaw, float pitch, float roll)
      : position(position), rotation(glm::vec3(pitch, yaw, roll))
  {
  }

  glm::vec3 get_position() const { return position; }
  glm::quat get_rotation() const { return rotation; }

  void set_position(const glm::vec3& new_position)
  {
    position = new_position;
  }

  void set_rotation(float yaw, float pitch, float roll)
  {
    rotation = glm::quat(glm::vec3(pitch, yaw, roll));
  }

protected:
  glm::vec3 position{};
  glm::quat rotation{};
};

} // namespace glhelp
