#pragma once

#include <unordered_map>
#include <unordered_set>

#include <glhelp/camera.hpp>
#include <glhelp/mesh/renderable.hpp>
#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/shader.hpp>

namespace glhelp {

class Scene {
public:
  Scene() = default;

  void add_object(const std::shared_ptr< Renderable >& renderable)
  {
    object_collection[renderable->get_shader()].insert(renderable);
  }

  template< PositionProvider CameraPositionSource >
  void draw_objects(Camera< CameraPositionSource >& camera, float time)
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

private:
  std::unordered_map< std::shared_ptr< ShaderProgram >, std::unordered_set< std::shared_ptr< Renderable > > > object_collection;
};

} // namespace glhelp
