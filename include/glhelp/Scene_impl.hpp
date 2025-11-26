#pragma once

// 'classic' header-guard to avoid recursive definition
#ifndef REC_SCENE_GUARD
#define REC_SCENE_GUARD

#include <glhelp/Scene.hpp>

namespace glhelp {

template< PositionProvider CameraPositionSource >
void Scene::draw_objects(const Camera< CameraPositionSource >& camera, float time)
{
  ShaderProgram::common_data.value().update_buffer({
      .camera_matrix = camera.get_projection_matrix() * camera.get_view_matrix(),
      .camera_position = glm::vec4(camera.get_position(), 0.0F),
      .time = time,
  });

  update_light_buffer();

  for (auto& [shader, objects] : object_collection) {
    shader->use();
    for (auto& object_ref : objects) {
      object_ref->draw();
    }
  }
}

template< PositionProvider CameraPositionSource >
void Scene::draw_minimap(const Camera< CameraPositionSource >& camera, const Window& window, float time, float scale, glm::vec2 position, glm::vec2 size)
{
  ShaderProgram::common_data.value().update_buffer({
      .camera_matrix = camera.get_ortho_matrix(size.x / size.y, scale) * camera.get_view_matrix(),
      .camera_position = glm::vec4(camera.get_position(), 0.0F),
      .time = time,
  });

  update_light_buffer();

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

} // namespace glhelp

#endif
