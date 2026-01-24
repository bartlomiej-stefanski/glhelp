#include "glhelp/position/CachingSimplePosition.hpp"
#include "glhelp/position/FPSPlayerController.hpp"
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

  const glm::vec3 start_point{0.0F};

  glhelp::CachingSimplePosition obj_position(glm::vec3{0.0F}, 0.0F, 0.0F, 0.0F);
  auto mesh(std::make_shared< glhelp::MeshObject< glhelp::CachingSimplePosition > >(glhelp::Obj< glhelp::VertexTextured >::parse_from_file(model_path), obj_position, mtl_shader));

  auto camera{std::make_shared< FPSCamera >(
      window,
      glhelp::FPSPlayerController(glhelp::FPSSimplePosition(start_point, glm::pi< float >(), 0, 0), 1, 1),
      90.0F, 0.001F, 100.0F)};

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

  auto scroll_event{window->scroll_event.connect([&](float, float yoffset) {
    camera->set_fov(glm::clamp(camera->get_fov() + yoffset, 55.0F, 120.0F));
  })};

  window->run_synchronously([&]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable -> bool {
    camera->poll_keys(window, frame_time);
    main_scene.draw_objects(*camera, time);
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
