#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/camera.hpp>
#include <glhelp/mesh/InstancedMesh3d.hpp>
#include <glhelp/mesh/mesh3d.hpp>
#include <glhelp/position/InteractiveController.hpp>
#include <glhelp/position/PlayerController.hpp>
#include <glhelp/position/FPSSimplePosition.hpp>
#include <glhelp/position/FPSPlayerController.hpp>
#include <glhelp/position/SimplePosition.hpp>
#include <glhelp/scene.hpp>
#include <glhelp/shader.hpp>
#include <glhelp/utils/glfw_context.hpp>
#include <glhelp/window.hpp>

#ifndef SHADER_DIR_PATH
#warning "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

void run_program()
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  std::vector< GLuint > shaders{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "fragment.glsl")};

  auto camera{std::make_shared< glhelp::Camera< glhelp::InteractiveController< glhelp::FPSPlayerController > > >(
    window,
    glhelp::FPSPlayerController(glhelp::FPSSimplePosition({0, 0, 5}, 0, 0, 0), 2, 1),
    90.0F, 0.1F, 100.0F
  )};

  auto default_shader{std::make_shared< glhelp::ShaderProgram >(std::move(shaders))};

  /*    7-----4
   *   /|   / |
   *  3----0  |
   *  | 6--|--5
   *  |/   | /
   *  2----1/
   */
  std::vector< glm::vec3 > triangle_vertices{
      glm::vec3(1, 1, 1),
      glm::vec3(1, -1, 1),
      glm::vec3(-1, -1, 1),
      glm::vec3(-1, 1, 1),

      glm::vec3(1, 1, -1),
      glm::vec3(1, -1, -1),
      glm::vec3(-1, -1, -1),
      glm::vec3(-1, 1, -1),
  };
  std::vector< uint > indices{
      // front
      0, 2, 1,
      2, 0, 3,
      // back
      5, 6, 7,
      5, 7, 4,
      // right
      0, 1, 4,
      1, 5, 4,
      // left
      2, 3, 6,
      6, 3, 7,
      // top
      0, 4, 7,
      0, 7, 3,
      // bottom
      1, 2, 5,
      2, 6, 5};

  constexpr auto instance_count = 4 * 4 * 4;
  std::vector< glm::vec3 > positions;
  positions.reserve(instance_count);
  for (unsigned x = 0; x < 4; x++) {
    for (unsigned y = 0; y < 4; y++) {
      for (unsigned z = 0; z < 4; z++) {
        positions.emplace_back(
            static_cast< float >(x) * 4.0F,
            static_cast< float >(y) * 4.0F,
            static_cast< float >(z) * 4.0F);
      }
    }
  }

  std::vector< float > brightness{};
  brightness.reserve(instance_count);
  for (size_t i = 0; i < positions.size(); i++) {
    brightness.emplace_back(glm::linearRand(0.1F, 1.0F));
  }

  glhelp::SimplePosition start_position{};
  start_position.set_scale({0.3F, 0.3F, 0.3F});
  auto cubes{std::make_shared< glhelp::InstancedMesh3d< glhelp::SimplePosition, glm::vec3, float > >(
      start_position, default_shader, triangle_vertices, indices, GL_TRIANGLES,
      std::tuple< std::vector< glm::vec3 >, std::vector< float > >{std::move(positions), std::move(brightness)})};

  glhelp::Scene main_scene;

  main_scene.add_object(cubes);
  camera->init_mouse(*window);

  window->run_synchronously([&main_scene, &camera, &cubes]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) {
    camera->poll_keys(window, static_cast< float >(frame_time));
    cubes->set_rotation(time, 0, 0);
    main_scene.draw_objects(*camera, time);
  });
}

auto main() -> int
{
  glhelp::GLFWContext context;
  run_program();
}
