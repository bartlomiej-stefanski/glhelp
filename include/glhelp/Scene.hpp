#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <glhelp/Camera.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/ligting/DirectionalLight.hpp>
#include <glhelp/ligting/SpotLight.hpp>
#include <glhelp/mesh/Renderable.hpp>

namespace glhelp {

class Scene {
public:
  Scene() = default;

  void add_object(const std::shared_ptr< Renderable >& renderable);
  void add_light(std::shared_ptr< DirectionalLight > light);
  void add_light(std::shared_ptr< SpotLight > light);

  template< PositionProvider CameraPositionSource >
  void draw_objects(const Camera< CameraPositionSource >& camera, float time);

  template< PositionProvider CameraPositionSource >
  void draw_minimap(const Camera< CameraPositionSource >& camera, const Window& window, float time, float scale, glm::vec2 position, glm::vec2 size);

private:
  void update_light_buffer() const;

  std::unordered_map< std::shared_ptr< ShaderProgram >, std::unordered_set< std::shared_ptr< Renderable > > > object_collection;
  std::unordered_set< std::shared_ptr< DirectionalLight > > directional_lights;
  std::unordered_set< std::shared_ptr< SpotLight > > spot_lights;
};

} // namespace glhelp

// Include template implementation.
#include <glhelp/Scene_impl.hpp>
