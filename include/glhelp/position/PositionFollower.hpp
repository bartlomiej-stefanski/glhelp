#pragma once

#include <cstdint>
#include <functional>

#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/SimplePosition.hpp>

namespace glhelp {

enum FollowOperation : std::uint8_t {
  PASSTHROUGH,
  ADD_OFFSET,
  ADD_OFFSET_WITH_ROTATION,
  OFFSET_ONLY,
};

/// Allows for following a PositionProvider object.
/// Implements PositionProvider interface.
template< PositionProvider PositionSource >
class PositionFollower {
public:
  PositionFollower(PositionSource& position_source, FollowOperation position_op = OFFSET_ONLY, FollowOperation rotation_op = OFFSET_ONLY)
      : position_op(position_op), rotation_op(rotation_op), position_source(position_source)
  {
  }

  PositionFollower(
      PositionSource& position_source,
      glm::vec3 offset,
      float yaw,
      float pitch,
      float roll,
      glm::vec3 scale,
      FollowOperation position_op = OFFSET_ONLY,
      FollowOperation rotation_op = OFFSET_ONLY)
      : position_op(position_op), rotation_op(rotation_op), offset(offset), rotation(glm::vec3(pitch, yaw, roll)), scale(scale), position_source(position_source)
  {
  }

  [[nodiscard]] auto get_position() const -> glm::vec3
  {
    switch (position_op) {
    case PASSTHROUGH:
      return position_source.get().get_position();
    case ADD_OFFSET:
      return position_source.get().get_position() + offset;
    case ADD_OFFSET_WITH_ROTATION:
      return position_source.get().get_position() + position_source.get().get_rotation() * offset;
    case OFFSET_ONLY:
    default:
      return offset;
    }
  }
  [[nodiscard]] auto get_rotation() const -> glm::quat
  {
    switch (rotation_op) {
    case PASSTHROUGH:
      return position_source.get().get_rotation();
    case ADD_OFFSET:
      return position_source.get().get_rotation() * rotation;
    case OFFSET_ONLY:
    default:
      return rotation;
    }
  }

  // Scale has no sense for PositionFollower.
  [[nodiscard]] auto get_scale() const -> glm::vec3 { return scale; }

  FollowOperation position_op;
  FollowOperation rotation_op;

  glm::vec3 offset{};
  glm::quat rotation{};
  glm::vec3 scale{1.0, 1.0, 1.0};

private:
  std::reference_wrapper< PositionSource > position_source;
};

} // namespace glhelp
