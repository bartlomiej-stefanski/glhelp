#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/Camera.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/mesh/Mesh2D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#ifndef SHADER_DIR_PATH
#warning "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

void run_program()
{
  auto window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 2d example")};

  std::vector< GLuint > shaders{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "fragment.glsl")};

  auto green_shader{std::make_shared< glhelp::ShaderProgram >(std::move(shaders))};

  std::vector< glm::vec2 > triangle_vertices{
      glm::vec2(-0.5F, -0.5F),
      glm::vec2(0.5F, -0.5F),
      glm::vec2(0.5F, 0.5F),
      glm::vec2(-0.5F, 0.5F),
  };

  glhelp::Mesh2D square(glhelp::SimplePosition{}, green_shader, triangle_vertices, {0, 1, 2, 2, 3, 0}, GL_TRIANGLES);

  auto main_loop = [&square](glhelp::Window& window, double time, double frame_time [[maybe_unused]]) {
    square.set_rotation(0, 0, time);
    square.draw(window.aspect_ratio());
  };

  window->run_synchronously(main_loop);
}

auto main() -> int
{
  glhelp::GLFWContext context;
  run_program();
}
