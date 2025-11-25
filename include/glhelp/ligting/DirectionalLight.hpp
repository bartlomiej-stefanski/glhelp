#pragma once

#include <glm/glm.hpp>

namespace glhelp {

/// Represents a simple directional light.
struct DirectionalLight {
  /// Normalized direction vector of the light.
  glm::vec3 direction;
  /// Color of the light.
  glm::vec3 color;
};

} // namespace glhelp
