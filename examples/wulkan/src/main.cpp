#include <algorithm>
#include <exception>
#include <glm/ext/quaternion_geometric.hpp>
#include <iostream>
#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <obj_parser/Obj.hpp>
#include <obj_parser/Vertex.hpp>

#include <glhelp/Camera.hpp>
#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/ligting/DirectionalLight.hpp>
#include <glhelp/ligting/SpotLight.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#include "Bubbles.hpp"
#include "Cube.hpp"
#include "Plane.hpp"
#include "glhelp/position/CachingSimplePosition.hpp"
#include "glhelp/position/SimplePosition.hpp"

#ifndef SHADER_DIR_PATH
#error "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

#ifndef OBJ_DIR_PATH
#error "Shader directory undefined. Please define OBJ_DIR_PATH macro."
#endif

void run_program(std::mt19937& rng, int n)
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  // Shader Instantiation
  auto phong_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "phong_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "phong_fragment.glsl")})};

  auto bubble_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "bubble_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "bubble_fragment.glsl")})};

  auto skybox_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "skybox_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "skybox_fragment.glsl")})};

  auto mountain_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "mountain_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "mountain_fragment.glsl")})};

  auto landing_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "landing_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "landing_fragment.glsl")})};
  auto landing_zone_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "landing_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "landing_zone_fragment.glsl")})};

  const glm::vec3 start_point{-450.0F, get_mountain_heigth(-450.0F, -450.0F), -450.0F};
  const glm::vec3 player_start{start_point + glm::vec3{0, 2.0F, 0}};

  // Object creation
  using CameraPosition = glhelp::InteractiveController< glhelp::FPSPlayerController >;
  auto camera{std::make_shared< glhelp::Camera< CameraPosition > >(
      window,
      glhelp::FPSPlayerController(glhelp::FPSSimplePosition(player_start, glm::pi< float >(), 0, 0), 5.0, 1),
      90.0F, 0.05F, 1000.0F)};

  auto icosphere_obj{obj_parser::Obj< obj_parser::VertexNormals >::parse_from_file(OBJ_DIR_PATH "icosphere.obj")};
  auto helicopter_landing_obj{obj_parser::Obj< obj_parser::VertexNormals >::parse_from_file(OBJ_DIR_PATH "helicopter_landing.obj")};
  auto helicopter_zone_obj{obj_parser::Obj< obj_parser::VertexNormals >::parse_from_file(OBJ_DIR_PATH "helicopter_zone.obj")};

  auto bubbles{std::make_shared< Bubbles >(
      bubble_shader,
      glhelp::SimplePosition(glm::vec3{0.0F, 0.0F, 0.0F}, 0, 0, 0, glm::vec3{1.0F}),
      icosphere_obj,
      get_bubble_data(n, rng))};

  auto mountain{std::make_shared< Plane >(
      glhelp::SimplePosition{},
      mountain_shader,
      1000)};

  using StaticMesh = glhelp::Mesh3D< glhelp::CachingSimplePosition >;
  auto landing_zone{std::make_shared< StaticMesh >(
      glhelp::CachingSimplePosition(start_point, 0, 0, 0, {1.0F, 1.0F, 1.0F}),
      landing_zone_shader,
      helicopter_zone_obj)};
  auto landing{std::make_shared< StaticMesh >(
      glhelp::CachingSimplePosition(start_point, 0, 0, 0, {1.0F, 1.0F, 1.0F}),
      landing_shader,
      helicopter_landing_obj)};

  auto skybox_position{glhelp::PositionFollower< CameraPosition >(
      *camera,
      glm::vec3{0.0},
      0, 0, 0,
      glm::vec3{1.0F, 1.0F, 1.0F} * 490.0F,
      true, false)};
  using Skybox = glhelp::Mesh3D< glhelp::PositionFollower< CameraPosition > >;
  auto skybox{std::make_shared< Skybox >(
      skybox_position,
      skybox_shader,
      cube_vertices,
      cube_indices_rev,
      GL_TRIANGLES)};

  auto sun{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
      .direction = glm::normalize(glm::vec3{-1.0f, -1.0f, -0.7f}),
      .color = glm::vec3{1.0f, 1.0f, 1.0f},
  })};

  auto flashlight{glhelp::MovingSpotLight< glhelp::PositionFollower< CameraPosition > >::create_shared(
      glhelp::PositionFollower< CameraPosition >(*camera, true, true),
      glm::vec3{1.0F, 1.0F, 0.6F},
      glm::radians(15.0F),
      glm::radians(30.0F))};

  glhelp::Scene main_scene;

  main_scene.add_object(skybox);
  main_scene.add_object(mountain);
  main_scene.add_object(landing_zone);
  main_scene.add_object(landing);

  main_scene.add_transparent_object(bubbles);

  main_scene.add_light(sun);
  // main_scene.add_light(flashlight);

  camera->init_mouse(*window);

  window->run_synchronously([&]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable {
    camera->poll_keys(window, static_cast< float >(frame_time));

    bubbles->update_order(camera->get_position());

    main_scene.draw_objects(*camera, time);
  });
}

static void print_help(const char* argv0)
{
  std::cerr << "Usage: " << argv0 << " [-n <number>] [-s <seed>]\n";
}

auto main(int argc, char* argv[]) -> int
{
  unsigned n{50};
  int seed{5}; // Uczciwy rzut kością!

  // 'paskudny-parser'
  for (int i{1}; i < argc; ++i) {
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

  try {
    glhelp::GLFWContext context;
    run_program(rng, n);
  }
  catch (std::exception& e) {
    std::cerr << "Program crashed!:\n"
              << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
