#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/Camera.hpp>
#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#include "cube.hpp"

#ifndef SHADER_DIR_PATH
#warning "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

void run_program(unsigned cube_side = 10)
{
  glm::vec3 obstacle_scale{glm::vec3{0.5F, 0.5F, 1.0F} / static_cast< float >(cube_side * 2)};

  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  std::vector< GLuint > obstacle_shaders{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "fragment.glsl")};
  auto obstacle_shader{std::make_shared< glhelp::ShaderProgram >(std::move(obstacle_shaders))};

  std::vector< GLuint > skybox_shaders{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "skybox_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "skybox_fragment.glsl")};
  auto skybox_shader{std::make_shared< glhelp::ShaderProgram >(std::move(skybox_shaders))};

  auto camera{std::make_shared< glhelp::Camera< glhelp::InteractiveController< glhelp::FPSPlayerController > > >(
      window,
      glhelp::FPSPlayerController(glhelp::FPSSimplePosition({0, 0, 0}, 0, 0, 0), 0.1, 1),
      90.0F, 0.001F, 100.0F)};

  auto position_follower{glhelp::PositionFollower< glhelp::Camera< glhelp::InteractiveController< glhelp::FPSPlayerController > > >(
      *camera,
      glm::vec3{0.0},
      0, 0, 0,
      glm::vec3{50.0F, 50.0F, 50.0F})};

  auto skybox{std::make_shared< glhelp::Mesh3D< glhelp::PositionFollower< glhelp::Camera< glhelp::InteractiveController< glhelp::FPSPlayerController > > > > >(
      position_follower,
      skybox_shader,
      cube_vertices,
      cube_indices_rev,
      GL_TRIANGLES)};

  std::vector< glm::mat4 > positions;
  positions.reserve(cube_side * cube_side * cube_side);
  for (unsigned x{}; x < cube_side; x++) {
    for (unsigned y{}; y < cube_side; y++) {
      for (unsigned z{}; z < cube_side; z++) {
        const glm::vec3 position{
            static_cast< float >(x) / static_cast< float >(cube_side - 1),
            static_cast< float >(y) / static_cast< float >(cube_side - 1),
            static_cast< float >(z) / static_cast< float >(cube_side - 1),
        };
        auto position_transform{glm::translate(glm::mat4{1.0F}, position)};
        auto scale_transform{glm::scale(glm::mat4{1.0F}, obstacle_scale)};
        positions.emplace_back(position_transform * scale_transform);
      }
    }
  }

  std::vector< float > brightness{};
  brightness.reserve(cube_side * cube_side * cube_side);
  for (size_t i = 0; i < positions.size(); i++) {
    brightness.emplace_back(glm::linearRand(0.1F, 1.0F));
  }

  glhelp::SimplePosition start_position{};
  start_position.set_scale({0.3F, 0.3F, 0.3F});
  auto cubes{std::make_shared< glhelp::InstancedMesh3d< glhelp::SimplePosition, glm::mat4 > >(
      start_position, obstacle_shader, cube_vertices, cube_indices, GL_TRIANGLES,
      std::tuple< std::vector< glm::mat4 > >{std::move(positions)})};

  glhelp::Scene main_scene;

  main_scene.add_object(skybox);
  main_scene.add_object(cubes);
  camera->init_mouse(*window);

  window->run_synchronously([&main_scene, &camera]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) {
    camera->poll_keys(window, static_cast< float >(frame_time));
    main_scene.draw_objects(*camera, time);
  });
}

auto main() -> int
{
  glhelp::GLFWContext context;
  run_program();
}
