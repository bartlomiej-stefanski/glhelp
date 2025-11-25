#pragma once

#include <glhelp/Shader.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>

#include "Cube.hpp"
#include "SphereObject.hpp"

using SkyboxPosition = glhelp::PositionFollower< FPSCamera >;

class Skybox : public glhelp::Mesh3D< SkyboxPosition > {
public:
  Skybox(std::shared_ptr< glhelp::ShaderProgram > shader, SkyboxPosition& position)
      : glhelp::Mesh3D< SkyboxPosition >(
            position,
            std::move(shader),
            cube_vertices,
            cube_indices_rev,
            GL_TRIANGLES)
  {
  }

  [[nodiscard]] auto get_wireframe_override() const noexcept -> bool override { return true; }

  void player_won(float time) { win_time = time; }

private:
  float win_time{};
  void uniform_setter_callback() override { shader->set_uniform("win_time", win_time); }
};
