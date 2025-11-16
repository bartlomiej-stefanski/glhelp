#include <iostream>
#include <memory>
#include <random>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glhelp/Camera.hpp>
#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#include "Obstacles.hpp"
#include "Skybox.hpp"
#include "SphereObject.hpp"

#ifndef SHADER_DIR_PATH
#warning "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

void run_program(unsigned cube_side, std::mt19937& dev)
{
  glm::vec3 obstacle_scale{0.8F * glm::vec3{0.5F, 0.5F, 1.0F} / static_cast< float >(cube_side)};

  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  auto obstacle_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "obstacle_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "obstacle_fragment.glsl")})};

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
  auto minimap_camera_position{glhelp::PositionFollower< FPSCamera >(
      *camera,
      glm::vec3{0.0F, 10.0F, 0.0F},
      0, glm::radians(-90.0F), 0,
      glm::vec3{1.0F, 1.0F, 1.0F},
      true, false)};

  auto player{std::make_shared< Player >(player_shader, player_position)};
  auto finish{std::make_shared< Finish >(player_shader, glhelp::CachingSimplePosition{glm::vec3{1, 1, 1}, 0, 0, 0, glm::vec3{0.02}})};
  auto skybox{std::make_shared< Skybox >(skybox_shader, skybox_position)};
  auto minimap_camera{std::make_shared< glhelp::Camera< glhelp::PositionFollower< FPSCamera > > >(
      window,
      minimap_camera_position,
      90.0F, 0.001F, 100.0F)};

  auto rombuses{std::make_shared< Rombuses >(
      obstacle_shader, glhelp::SimplePosition{}, get_rhombus_positions(cube_side, obstacle_scale, dev))};

  glhelp::Scene main_scene;

  main_scene.add_object(skybox);
  main_scene.add_object(rombuses);
  main_scene.add_object(player);
  main_scene.add_object(finish);
  camera->init_mouse(*window);

  window->run_synchronously([&, won = false]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable {
    const auto old_position{camera->get_position()};
    camera->poll_keys(window, static_cast< float >(frame_time));
    if (rombuses->collides_with(camera->get_position(), 0.005)) {
      camera->set_position(old_position); // Do not move player if it would cause a collision
    }

    const auto finish_position{finish->get_position()};
    if (glm::distance(camera->get_position(), finish_position) <= finish->get_radius() * 1.2F) {
      if (!won) {
        std::cout << "Wygrano w czasie: " << time << '\n';
        skybox->player_won(time);
        won = true;
      }
      camera->set_position(old_position);
    }

    main_scene.draw_objects(*camera, time);
    main_scene.draw_minimap(*minimap_camera, window, time, 0.2F, {0, 0}, {window.get_size().x * 0.2F, window.get_size().x * 0.2F});
  });
}

void print_help(const char* argv0)
{
  std::cerr << "Usage: " << argv0 << " [-n <number>] [-s <seed>]\n";
}

auto main(int argc, char* argv[]) -> int
{
  unsigned n = 10;
  int seed = 5; // Uczciwy rzut kością!

  // 'paskudny-parser'
  for (int i = 1; i < argc; ++i) {
    if (argv[i] == std::string("-n") && i + 1 < argc) {
      n = std::stoi(argv[++i]);
    }
    else if (argv[i] == std::string("-s") && i + 1 < argc) {
      seed = std::stoi(argv[++i]);
    }
    else if (argv[i] == std::string("-h")) {
      print_help(argv[0]);
      return 0;
    }
    else {
      std::cerr << "Unknown option: " << argv[i] << '\n';
      print_help(argv[0]);
      return 1;
    }
  }

  std::mt19937 rng(seed);

  glhelp::GLFWContext context;

  run_program(n, rng);
}
