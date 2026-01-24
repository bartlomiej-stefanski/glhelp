#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glhelp/Buttons.hpp>
#include <glhelp/Camera.hpp>
#include <glhelp/Obj.hpp>
#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Vertex.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/ligting/DirectionalLight.hpp>
#include <glhelp/ligting/SpotLight.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/Event.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#include "GlobeCamera.hpp"

#ifndef SHADER_DIR_PATH
#error "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

#ifndef OBJ_DIR_PATH
#error "Shader directory undefined. Please define OBJ_DIR_PATH macro."
#endif

namespace fs = std::filesystem;

using FPSCamera = glhelp::Camera< glhelp::InteractiveController< glhelp::FPSPlayerController > >;

void run_program(const fs::path& model_path)
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "3D Object Viewer")};

  auto mtl_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "mtl_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "mtl_fragment.glsl")})};

  glhelp::CachingSimplePosition obj_position(glm::vec3{0.0F}, 0.0F, 0.0F, 0.0F);
  auto mesh(std::make_shared< glhelp::MeshObject< glhelp::CachingSimplePosition > >(glhelp::Obj< glhelp::VertexTextured >::parse_from_file(model_path), obj_position, mtl_shader));

  std::cerr << std::format("Mesh radius: {}\n", mesh->radius);
  auto globe_camera{std::make_shared< GlobeCamera >(window, GlobePosition(glm::vec2{0.0F}, 0.5F, mesh->radius))};
  globe_camera->init_mouse(*window);

  auto sun{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
      .direction = glm::normalize(glm::vec3{-1.0F, -1.0F, -1.0F}),
      .color = glm::vec3{1.1F, 1.1F, 1.0F},
  })};
  auto top_light{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
      .direction = glm::normalize(glm::vec3{1.0F, -1.0F, 1.0F}),
      .color = glm::vec3{0.1F, 0.1F, 0.1F},
  })};

  glhelp::Scene main_scene;
  main_scene.add_object(mesh);

  main_scene.add_light(sun);
  main_scene.add_light(top_light);

  window->run_synchronously([&]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable -> bool {
    globe_camera->poll_keyboard_events(window, frame_time);
    // auto globe_pos{globe_camera->get_position()};
    // std::cerr << std::format("{}, {}, {}\n", globe_pos.x, globe_pos.y, globe_pos.z);
    main_scene.draw_objects(*globe_camera, time);
    return true;
  });
}

static void print_help(const char* argv0)
{
  std::cerr << "Usage: " << argv0 << " <model path>\n"
            << "  <model path> : Path to .obj model file\n";
}

auto main(int argc, char* argv[]) -> int
{
  if (argc == 1) {
    std::cerr << "Incorrect argument set!\n";
    print_help(argv[0]);
    return 1;
  }

  fs::path file_path{argv[1]};
  if (!fs::exists(file_path)) {
    std::cerr << "Incorrect file path provided! No model found for: " << file_path.string() << "\n";
    return 1;
  }

  try {
    glhelp::GLFWContext context;
    run_program(file_path);
  }
  catch (std::exception& e) {
    std::cerr << "Program crashed!:\n"
              << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
