#include <iostream>
#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/glfw_context.hpp>
#include <glhelp/primitives/PlayerController.hpp>
#include <glhelp/primitives/SimplePosition.hpp>
#include <glhelp/primitives/camera.hpp>
#include <glhelp/primitives/mesh3d.hpp>
#include <glhelp/shader.hpp>
#include <glhelp/window.hpp>

void run_program()
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  std::vector< GLuint > shaders{glhelp::create_shader(GL_VERTEX_SHADER,
#include STR(vertex.glsl)
                                                      ),
                                glhelp::create_shader(GL_FRAGMENT_SHADER,
#include STR(fragment.glsl)
                                                      )};

  glhelp::Camera< glhelp::PlayerController > camera(
      window,
      glhelp::PlayerController(glm::vec3{0, 0, 1}, 0, 0, 0),
      90.0f,
      0.1,
      100.0);

  std::shared_ptr< glhelp::ShaderProgram > green_shader{
      std::make_shared< glhelp::ShaderProgram >(std::move(shaders))};

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

  auto mouse_delegate = window->mouse_event.new_delegate([&camera](float xoffset, float yoffset) {
    camera.look_up(yoffset / 200.0);
    camera.look_right(-xoffset / 200.0);
  });
  window->mouse_event.connect(mouse_delegate);

  glhelp::Mesh3D cube(glhelp::SimplePosition{}, green_shader, triangle_vertices, indices, GL_TRIANGLES);

  auto main_loop = [&camera, &cube, time = 0.0f]([[maybe_unused]] glhelp::Window& window, double frame_time) mutable {
    if (glfwGetKey(window.get_window(), GLFW_KEY_W) == GLFW_PRESS) {
      camera.move_forwards(-2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_S) == GLFW_PRESS) {
      camera.move_forwards(2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_D) == GLFW_PRESS) {
      camera.strafe(2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_A) == GLFW_PRESS) {
      camera.strafe(-2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_SPACE) == GLFW_PRESS) {
      camera.move_up(2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
      camera.move_up(-2.0f * frame_time);
    }

    if (glfwGetKey(window.get_window(), GLFW_KEY_UP) == GLFW_PRESS) {
      camera.look_up(2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_DOWN) == GLFW_PRESS) {
      camera.look_up(-2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
      camera.look_right(-2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
      camera.look_right(2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_E) == GLFW_PRESS) {
      camera.roll_cc(-2.0f * frame_time);
    }
    if (glfwGetKey(window.get_window(), GLFW_KEY_Q) == GLFW_PRESS) {
      camera.roll_cc(2.0f * frame_time);
    }

    cube.set_rotation(time, 0, 0);
    cube.draw(camera);
    time += frame_time;
  };

  window->run_synchronously(main_loop);
}

int main()
{
  glhelp::GLFWContext context;
  run_program();
}
