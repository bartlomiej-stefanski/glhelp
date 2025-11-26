#pragma once

#include <memory>
#include <optional>

#include <glm/glm.hpp>

#include <glhelp/position/PositionProvider.hpp>

namespace glhelp {

/// Abstract representation of a spotlight.
struct SpotLight {
  [[nodiscard]] virtual auto get_position() const noexcept -> glm::vec3 = 0;
  [[nodiscard]] virtual auto get_direction() const noexcept -> glm::vec3 = 0;
  virtual ~SpotLight() = default;

  /// Light color. Serves as intensity modifier.
  glm::vec3 color;

  float linear_coeff{};
  float quadratic_coeff{};

  /// Inner light cone cutoff angle in radians.
  float cutoff;
  /// Outer light cone cutoff angle in radians.
  /// If set allows for smooth transition between full intensity and no intensity.
  std::optional< float > outer_cutoff;
};

/// Representation of a spotlight with a dynamic position and direction.
template< PositionProvider PositionSource >
class MovingSpotLight : public SpotLight {
public:
  MovingSpotLight(
      PositionSource&& position_source,
      glm::vec3 color,
      float cutoff = glm::pi< float >(),
      std::optional< float > outer_cutoff = std::nullopt)
      : position_source(std::move(position_source))
  {
    this->color = color;
    this->cutoff = cutoff;
    this->outer_cutoff = outer_cutoff;
  }

  [[nodiscard]] auto get_position() const noexcept -> glm::vec3 override
  {
    return position_source.get_position();
  }

  [[nodiscard]] auto get_direction() const noexcept -> glm::vec3 override
  {
    return position_source.get_rotation() * glm::vec3{0, 0, 1};
  }

  static auto create_shared(
      PositionSource&& position_source,
      glm::vec3 color,
      float cutoff = glm::pi< float >(),
      std::optional< float > outer_cutoff = std::nullopt) -> std::shared_ptr< SpotLight >
  {
    return std::make_shared< MovingSpotLight< PositionSource > >(std::move(position_source), color, cutoff, outer_cutoff);
  }

private:
  PositionSource position_source;
};

} // namespace glhelp
