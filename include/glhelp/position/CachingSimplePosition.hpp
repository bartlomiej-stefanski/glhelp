#pragma once

#include <optional>

#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

class CachingSimplePosition {
public:
  CachingSimplePosition() = default;

  CachingSimplePosition(glm::vec3 position, float yaw, float pitch, float roll, glm::vec3 scale = glm::vec3{1.0F})
      : position(position), rotation(glm::vec3(pitch, yaw, roll)), scale(scale)
  {
  }

  [[nodiscard]] auto get_position() const -> glm::vec3 { return position; }
  [[nodiscard]] auto get_rotation() const -> glm::quat { return rotation; }
  [[nodiscard]] auto get_scale() const -> glm::vec3 { return scale; }

  [[nodiscard]] auto get_cached() const -> std::optional< glm::mat4 >
  {
    return cached_model_matrix;
  }

  auto cache(glm::mat4&& model_matrix) const -> glm::mat4
  {
    cached_model_matrix.emplace(model_matrix);
    return *cached_model_matrix;
  }

  void set_position(const glm::vec3& new_position)
  {
    position = new_position;
    cached_model_matrix.reset();
  }

  void set_rotation(float yaw, float pitch, float roll)
  {
    rotation = glm::quat(glm::vec3(pitch, yaw, roll));
    cached_model_matrix.reset();
  }

  void set_scale(const glm::vec3& new_scale)
  {
    scale = new_scale;
    cached_model_matrix.reset();
  }

protected:
  glm::vec3 position{};
  glm::quat rotation{};
  glm::vec3 scale{1.0};
  mutable std::optional< glm::mat4 > cached_model_matrix{};
};

} // namespace glhelp
