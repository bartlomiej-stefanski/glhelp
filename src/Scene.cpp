#include <glhelp/Scene.hpp>

namespace glhelp {

void Scene::add_object(const std::shared_ptr< Renderable >& renderable)
{
  object_collection[renderable->get_shader()].insert(renderable);
}

void Scene::add_light(std::shared_ptr< DirectionalLight > light)
{
  if (directional_lights.size() >= MAX_DIRECTIONAL_LIGHTS) {
    throw std::runtime_error("Exceeded MAX_DIRECTIONA_LIGHTS in the scene");
  }

  directional_lights.insert(std::move(light));
}

void Scene::add_light(std::shared_ptr< SpotLight > light)
{
  if (spot_lights.size() >= MAX_SPOT_LIGHTS) {
    throw std::runtime_error("Exceeded MAX_SPOT_LIGHTS in the scene");
  }

  spot_lights.insert(std::move(light));
}

void Scene::update_light_buffer() const
{
  {
    ShaderProgram::DirectionalLightData dir_light_data{};
    for (const auto& light : directional_lights) {
      const auto inx{dir_light_data.count++};
      dir_light_data.color[inx] = glm::vec4{light->color, 1.0f};
      dir_light_data.direction[inx] = glm::vec4{light->direction, 0.0f};
    }
    ShaderProgram::directional_light_data->update_buffer(dir_light_data);
  }
  {
    ShaderProgram::SpotLightData spot_light_data{};
    for (const auto& light : spot_lights) {
      const auto inx{spot_light_data.count++};
      spot_light_data.color[inx] = glm::vec4{light->color, 1.0f};
      spot_light_data.direction[inx] = glm::vec4{light->get_direction(), 0.0f};
      spot_light_data.position[inx] = glm::vec4{light->get_position(), 1.0f};
      spot_light_data.cutoff[inx] = light->cutoff;
      spot_light_data.outer_cutoff[inx] = light->outer_cutoff.value_or(light->cutoff);
    }
    ShaderProgram::spot_light_data->update_buffer(spot_light_data);
  }
}

} // namespace glhelp
