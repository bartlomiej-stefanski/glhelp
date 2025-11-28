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

#ifndef SHADER_DIR_PATH
#error "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

#ifndef OBJ_DIR_PATH
#error "Shader directory undefined. Please define OBJ_DIR_PATH macro."
#endif

void run_program(std::mt19937& rng, int n)
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  auto phong_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "phong_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "phong_fragment.glsl")})};

  auto bubble_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "bubble_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "bubble_fragment.glsl")})};

  using CameraPosition = glhelp::InteractiveController< glhelp::FPSPlayerController >;
  auto camera{std::make_shared< glhelp::Camera< CameraPosition > >(
      window,
      glhelp::FPSPlayerController(glhelp::FPSSimplePosition({-0.03F, -0.03F, -0.03F}, glm::pi< float >(), 0, 0), 0.3, 1),
      90.0F, 0.001F, 100.0F)};

  auto suzane_obj{obj_parser::Obj< obj_parser::VertexNormals >::parse_from_file(OBJ_DIR_PATH "suzane.obj")};
  auto icosphere_obj{obj_parser::Obj< obj_parser::VertexNormals >::parse_from_file(OBJ_DIR_PATH "icosphere.obj")};

  auto suzane{std::make_shared< glhelp::Mesh3D< glhelp::SimplePosition > >(
      glhelp::SimplePosition{glm::vec3{0.2f, 0, 0}, 0, 0, 0, glm::vec3{0.1}}, phong_shader, suzane_obj)};

  auto bubbles{std::make_shared< Bubbles >(
      bubble_shader,
      glhelp::SimplePosition(glm::vec3{0.0F, 0.0F, 0.0F}, 0, 0, 0, glm::vec3{0.06F}),
      icosphere_obj,
      get_bubble_positions(n, rng),
      get_bubble_info(n, rng))};

  glhelp::Scene main_scene;

  auto sun{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
      .direction = glm::normalize(glm::vec3{-1.0f, -1.0f, -0.5f}),
      .color = glm::vec3{1.0f, 1.0f, 0.2f},
  })};
  auto moon{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
      .direction = glm::normalize(glm::vec3{1.0f, -1.0f, -0.5f}),
      .color = glm::vec3{0.2, 0.2, 0.9F},
  })};

  auto flashlight{glhelp::MovingSpotLight< glhelp::PositionFollower< CameraPosition > >::create_shared(
      glhelp::PositionFollower< CameraPosition >(*camera, true, true),
      glm::vec3{1.0F, 1.0F, 0.6F},
      glm::radians(15.0F),
      glm::radians(30.0F))};

  main_scene.add_object(suzane);

  main_scene.add_transparent_object(bubbles);

  main_scene.add_light(sun);
  main_scene.add_light(moon);
  main_scene.add_light(flashlight);

  camera->init_mouse(*window);

  window->run_synchronously([&]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable {
    camera->poll_keys(window, static_cast< float >(frame_time));
    suzane->set_rotation(time / 10, 0, 0);

    main_scene.draw_objects(*camera, time);
  });
}

static void print_help(const char* argv0)
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
