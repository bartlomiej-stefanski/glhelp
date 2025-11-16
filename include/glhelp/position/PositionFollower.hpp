#pragma once

#include <functional>

#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/SimplePosition.hpp>

namespace glhelp {

/// Allows for following a PositionProvider object.
/// Implements PositionProvider interface.
template< PositionProvider PositionSource >
class PositionFollower {
public:
  PositionFollower(PositionSource& position_source, bool passthrough_possition = false, bool pasthrough_rotation = false)
      : position_source(position_source), passthrough_possition(passthrough_possition), pasthrough_rotation(pasthrough_rotation)
  {
  }

  PositionFollower(
      PositionSource& position_source,
      glm::vec3 offset,
      float yaw,
      float pitch,
      float roll,
      glm::vec3 scale,
      bool passthrough_possition = false,
      bool pasthrough_rotation = false)
      : offset(offset), rotation(glm::vec3(pitch, yaw, roll)), scale(scale), position_source(position_source),
        passthrough_possition(passthrough_possition), pasthrough_rotation(pasthrough_rotation)
  {
  }

  [[nodiscard]] auto get_position() const -> glm::vec3
  {
    if (passthrough_possition) {
      return position_source.get().get_position() + offset;
      ;
    }

    return offset;
  }
  [[nodiscard]] auto get_rotation() const -> glm::quat
  {
    if (pasthrough_rotation) {
      return position_source.get().get_rotation();
    }

    return rotation;
  }

  // Scale has no sense for PositionFollower.
  [[nodiscard]] auto get_scale() const -> glm::vec3 { return scale; }

private:
  glm::vec3 offset{};
  glm::quat rotation{};
  glm::vec3 scale{1.0, 1.0, 1.0};

  std::reference_wrapper< PositionSource > position_source;
  bool passthrough_possition{};
  bool pasthrough_rotation{};
};

} // namespace glhelp
