#pragma once

#include <memory>

#include <glhelp/Camera.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>

#include "Sphere.hpp"

using FPSCamera = glhelp::Camera< glhelp::InteractiveController< glhelp::FPSPlayerController > >;
using PlayerPositionSource = glhelp::PositionFollower< FPSCamera >;

template< glhelp::PositionProvider PositionSource >
class SphereObject : public glhelp::Mesh3D< PositionSource > {
public:
  SphereObject(std::shared_ptr< glhelp::ShaderProgram > shader, PositionSource fps_camera)
      : glhelp::Mesh3D< PositionSource >(
            std::move(fps_camera),
            std::move(shader),
            player_sphere.vertices,
            player_sphere.indices,
            GL_TRIANGLES)
  {
  }

  [[nodiscard]] auto get_radius() const noexcept -> float { return this->get_scale().x; }

  [[nodiscard]] auto get_wireframe_override() const noexcept -> bool override { return true; }
};

using Player = SphereObject< PlayerPositionSource >;
using Finish = SphereObject< glhelp::CachingSimplePosition >;
