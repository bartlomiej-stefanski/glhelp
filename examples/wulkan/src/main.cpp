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
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#ifndef SHADER_DIR_PATH
#error "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

#ifndef OBJ_DIR_PATH
#error "Shader directory undefined. Please define OBJ_DIR_PATH macro."
#endif

void run_program()
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  auto diffuse_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "diffuse_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "diffuse_fragment.glsl")})};

  auto camera{std::make_shared< glhelp::Camera< glhelp::InteractiveController< glhelp::FPSPlayerController > > >(
      window,
      glhelp::FPSPlayerController(glhelp::FPSSimplePosition({-0.03F, -0.03F, -0.03F}, glm::pi< float >(), 0, 0), 0.3, 1),
      90.0F, 0.001F, 100.0F)};

  auto suzane{obj_parser::Obj< obj_parser::VertexNormals >::parse_from_file(OBJ_DIR_PATH "suzane.obj")};

  auto finish{std::make_shared< glhelp::Mesh3D< glhelp::SimplePosition > >(
      glhelp::SimplePosition{glm::vec3{0.2f, 0, 0}, 0, 0, 0, glm::vec3{0.1}}, diffuse_shader, suzane)};

  glhelp::Scene main_scene;

  auto sun{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
      .direction = glm::normalize(glm::vec3{1.0f, 1.0f, 0.5f}),
      .color = glm::vec3{1.0f, 1.0f, 0.9f},
  })};
  auto moon{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
      .direction = glm::normalize(glm::vec3{-1.0f, 1.0f, 0.5f}),
      .color = glm::vec3{0.6, 0.6, 0.9F},
  })};

  main_scene.add_object(finish);
  main_scene.add_light(sun);
  main_scene.add_light(moon);
  camera->init_mouse(*window);

  window->run_synchronously([&]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable {
    camera->poll_keys(window, static_cast< float >(frame_time));
    main_scene.draw_objects(*camera, time);
  });
}

auto main() -> int
{
  try {
    glhelp::GLFWContext context;
    run_program();
  }
  catch (std::exception& e) {
    std::cerr << "Program crashed!:\n"
              << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
