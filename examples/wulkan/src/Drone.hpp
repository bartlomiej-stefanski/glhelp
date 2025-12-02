#pragma once

#include <functional>
#include <glm/glm.hpp>

#include <obj_parser/Vertex.hpp>

#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>

#include "HealthBar.hpp"
#include "glhelp/position/PositionFollower.hpp"

using DronePosition = glhelp::InteractiveController< glhelp::FPSPlayerController >;

class Drone : public glhelp::Mesh3D< DronePosition > {
public:
  Drone(
      DronePosition position_source,
      std::shared_ptr< glhelp::ShaderProgram > shader,
      const obj_parser::Obj< obj_parser::VertexNormals >& obj,
      std::shared_ptr< glhelp::ShaderProgram > rotor_shader,
      const obj_parser::Obj< obj_parser::VertexNormals >& rotor_obj,
      std::shared_ptr< glhelp::ShaderProgram > health_bar_shader,
      const obj_parser::Obj< obj_parser::SimpleVertex >& health_bar_obj)
      : glhelp::Mesh3D< DronePosition >(std::move(position_source), std::move(shader), obj)
  {
    glm::vec3 offset{-2.89783F, 0.0F, -2.68395F};

    std::vector< glm::vec3 > rotor_offsets{
        0.1F * offset,
        0.1F * glm::vec3{offset.x, offset.y, -offset.z},
        0.1F * -offset,
        0.1F * glm::vec3{-offset.x, offset.y, offset.z},
    };

    for (auto i{0}; i < 4; i++) {
      rotors[i].emplace(std::make_shared< glhelp::Mesh3D< glhelp::PositionFollower< DronePosition > > >(
          glhelp::PositionFollower< DronePosition >(
              *this,
              rotor_offsets[i],
              0, 0, 0,
              glm::vec3{0.1F},
              glhelp::ADD_OFFSET_WITH_ROTATION,
              glhelp::ADD_OFFSET),
          rotor_shader,
          rotor_obj));
    }

    health_bar.emplace(std::make_shared< HealthBar >(
        DroneFollowPosition(
            *this,
            glm::vec3{0.0F, 0.0F, 0.263F},
            0, 0, 0,
            glm::vec3{0.07F, 0.01F, 0.01F},
            glhelp::ADD_OFFSET_WITH_ROTATION,
            glhelp::ADD_OFFSET),
        std::move(health_bar_shader),
        health_bar_obj));
  }

  void rotate_rotors(float time)
  {
    for (auto rotor : rotors) {
      rotor.value()->rotation = glm::angleAxis(-50.0F * time, glhelp::UP_VECTOR);
    }
  }

  void add_to_scene(glhelp::Scene& scene)
  {
    // Iterative for-loop segfaults (no idea why!)
    for (int i{}; i < 4; i++)
      scene.add_object(rotors[i].value());

    scene.add_object(health_bar.value());
  }

  auto check_terrain_collision(const std::function< bool(glm::vec3, float) >& predicate) const -> bool
  {
    for (int i{}; i < 4; i++) {
      if (predicate(rotors[i].value()->get_position(), 0.2F)) {
        return true;
      }
    }

    return predicate(this->get_position(), 0.2F);
  }

  std::array< std::optional< std::shared_ptr< glhelp::Mesh3D< glhelp::PositionFollower< DronePosition > > > >, 4 > rotors{};
  std::optional< std::shared_ptr< HealthBar > > health_bar{};
};
