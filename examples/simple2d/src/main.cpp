#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/camera.hpp>
#include <glhelp/mesh/mesh2d.hpp>
#include <glhelp/position/SimplePosition.hpp>
#include <glhelp/shader.hpp>
#include <glhelp/utils/glfw_context.hpp>
#include <glhelp/window.hpp>

void run_program()
{
  auto window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 2d example")};

  std::vector< GLuint > shaders{glhelp::create_shader(GL_VERTEX_SHADER,
#include STR(vertex.glsl)
                                                      ),
                                glhelp::create_shader(GL_FRAGMENT_SHADER,
#include STR(fragment.glsl)
                                                      )};

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
