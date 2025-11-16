#pragma once

#include <unordered_map>
#include <unordered_set>

#include <glhelp/Camera.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/mesh/Renderable.hpp>
#include <glhelp/position/Position.hpp>

namespace glhelp {

class Scene {
public:
  Scene() = default;

  void add_object(const std::shared_ptr< Renderable >& renderable)
  {
    object_collection[renderable->get_shader()].insert(renderable);
  }

  template< PositionProvider CameraPositionSource >
  void draw_objects(const Camera< CameraPositionSource >& camera, float time)
  {
    ShaderProgram::common_data.value().update_buffer({
        .camera_matrix = camera.get_projection_matrix() * camera.get_view_matrix(),
        .time = time,
    });

    for (auto& [shader, objects] : object_collection) {
      shader->use();
      for (auto& object_ref : objects) {
        object_ref->draw();
      }
    }
  }

  template< PositionProvider CameraPositionSource >
  void draw_minimap(const Camera< CameraPositionSource >& camera, const Window& window, float time, float scale, glm::vec2 position, glm::vec2 size)
  {
    ShaderProgram::common_data.value().update_buffer({
        .camera_matrix = camera.get_ortho_matrix(size.x / size.y, scale) * camera.get_view_matrix(),
        .time = time,
    });

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClear(GL_DEPTH_BUFFER_BIT);

    glViewport(position.x, position.y, size.x, size.y);

    for (auto& [shader, objects] : object_collection) {
      shader->use();
      for (auto& object_ref : objects) {
        if (object_ref->get_wireframe_override()) {
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          object_ref->draw();
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
          object_ref->draw();
        }
      }
    }

    glViewport(0, 0, static_cast< int >(window.get_size().x), static_cast< int >(window.get_size().y));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

private:
  std::unordered_map< std::shared_ptr< ShaderProgram >, std::unordered_set< std::shared_ptr< Renderable > > > object_collection;
};

} // namespace glhelp
