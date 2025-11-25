#include <iostream>
#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <obj_parser/Obj.hpp>

#include <glhelp/Camera.hpp>
#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/GLFWContext.hpp>
#include <glhelp/position/CachingSimplePosition.hpp>

#include "Skybox.hpp"
#include "SphereObject.hpp"
#include "obj_parser/Vertex.hpp"

#ifndef SHADER_DIR_PATH
#warning "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

void run_program()
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  auto player_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "player_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "player_fragment.glsl")})};

  auto skybox_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "skybox_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "skybox_fragment.glsl")})};

  auto camera{std::make_shared< FPSCamera >(
      window,
      glhelp::FPSPlayerController(glhelp::FPSSimplePosition({-0.03F, -0.03F, -0.03F}, glm::pi< float >(), 0, 0), 0.1, 1),
      90.0F, 0.001F, 100.0F)};

  auto skybox_position{glhelp::PositionFollower< FPSCamera >(
      *camera,
      glm::vec3{0.0},
      0, 0, 0,
      glm::vec3{50.0F, 50.0F, 50.0F},
      true, false)};
  auto player_position{glhelp::PositionFollower< FPSCamera >(
      *camera,
      glm::vec3{0.0},
      0, 0, 0,
      glm::vec3{0.01F, 0.01F, 0.01F},
      true, false)};

  auto monkey{obj_parser::Obj< obj_parser::SimpleVertex >::parse_from_file(SHADER_DIR_PATH "suzane.obj")};

  auto player{std::make_shared< Player >(player_shader, player_position)};
  auto finish{std::make_shared< glhelp::Mesh3D< glhelp::CachingSimplePosition> >(
    glhelp::CachingSimplePosition{glm::vec3{1, 1, 1}, 0, 0, 0, glm::vec3{0.02}}, player_shader, monkey
  )};
  auto skybox{std::make_shared< Skybox >(skybox_shader, skybox_position)};

  glhelp::Scene main_scene;

  main_scene.add_object(skybox);
  main_scene.add_object(player);
  main_scene.add_object(finish);
  camera->init_mouse(*window);

  window->run_synchronously([&]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable {
    camera->poll_keys(window, static_cast< float >(frame_time));
    main_scene.draw_objects(*camera, time);
  });
}

auto main() -> int
{
  glhelp::GLFWContext context;
  run_program();
}
