#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/glfw_context.hpp>
#include <glhelp/primitives/SimplePosition.hpp>
#include <glhelp/primitives/camera.hpp>
#include <glhelp/primitives/mesh2d.hpp>
#include <glhelp/shader.hpp>
#include <glhelp/window.hpp>

void run_program()
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 2d example")};

  std::vector< GLuint > shaders{glhelp::create_shader(GL_VERTEX_SHADER,
#include STR(vertex.glsl)
                                                      ),
                                glhelp::create_shader(GL_FRAGMENT_SHADER,
#include STR(fragment.glsl)
                                                      )};

  glhelp::Camera< glhelp::SimplePosition > camera(
      window,
      glhelp::SimplePosition(glm::vec3{0, 0, -1}, 0, 0, 0),
      45.0f,
      0.1,
      100.0);

  std::shared_ptr< glhelp::ShaderProgram > green_shader{
      std::make_shared< glhelp::ShaderProgram >(std::move(shaders))};

  std::vector< glm::vec2 > triangle_vertices{
      glm::vec2(-0.5f, -0.5f),
      glm::vec2(0.5f, -0.5f),
      glm::vec2(0.5f, 0.5f),
      glm::vec2(-0.5f, 0.5f),
  };

  glhelp::Mesh2D< glhelp::SimplePosition > square(green_shader, triangle_vertices, {0, 1, 2, 2, 3, 0}, GL_TRIANGLES);

  auto main_loop = [&square, time = 0.0f](glhelp::Window& window, double frame_time) mutable {
    square.roll = time * 90.0f;
    square.draw(window.aspect_ratio());
    time += frame_time;
  };

  window->run_synchronously(main_loop);
}

int main()
{
  glhelp::GLFWContext context;
  run_program();
}
