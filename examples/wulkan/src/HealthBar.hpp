#pragma once

#include <memory>

#include <glhelp/Obj.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>

using DroneFollowPosition = glhelp::PositionFollower< glhelp::InteractiveController< glhelp::FPSPlayerController > >;

class HealthBar : public glhelp::Mesh3D< DroneFollowPosition > {
public:
  HealthBar(DroneFollowPosition position, std::shared_ptr< glhelp::ShaderProgram > shader, const glhelp::Obj< glhelp::SimpleVertex >& bar)
      : glhelp::Mesh3D< DroneFollowPosition >(
            position,
            std::move(shader),
            bar),
        initial_scale_x(position.scale.x)
  {
  }

  void record_collision()
  {
    health -= 0.49F;
  }

  void gas_damage(float frame_time, float distance)
  {
    health -= 1.0F * frame_time / distance;
  }

  void distance_damage(float frame_time)
  {
    health -= 0.25F * frame_time;
  }

  [[nodiscard]] auto is_alive() const noexcept -> bool { return health > 0.0F; }

  void uniform_setter_callback() override
  {
    scale.x = glm::max(initial_scale_x * health, 0.0F);
    shader->set_uniform("uHealth", health);
  }

private:
  float initial_scale_x;
  float health{1.0F};
};
