#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Sphere.hpp"

auto generate_sphere(unsigned sector_count, unsigned stack_count) -> Sphere
{
  Sphere mesh;

  mesh.vertices.emplace_back(0.0F, 1.0F, 0.0F);
  for (unsigned i = 1; i < stack_count; i++) {
    const float pitch_angle = glm::pi< float >() * i / stack_count;

    for (unsigned j = 0; j <= sector_count; j++) {
      const float yaw_angle = glm::two_pi< float >() * j / sector_count;
      const float x = sinf(pitch_angle) * cosf(yaw_angle);
      const float y = cosf(pitch_angle);
      const float z = sinf(pitch_angle) * sinf(yaw_angle);
      mesh.vertices.emplace_back(x, y, z);
    }
  }
  mesh.vertices.emplace_back(0.0F, -1.0F, 0.0F);

  // Conect north and south pole
  for (unsigned i = 1; i <= sector_count; i++) {
    // North pole
    mesh.indices.push_back(0);
    mesh.indices.push_back(i + 1);
    mesh.indices.push_back(i);
    // South pole
    const auto south_pole_index = mesh.vertices.size() - 1;
    mesh.indices.push_back(south_pole_index);
    mesh.indices.push_back(south_pole_index - i - 1);
    mesh.indices.push_back(south_pole_index - i);
  }

  // Connect stacks
  for (unsigned i = 0; i < stack_count - 2; i++) {
    const auto stack_start_index = 1 + i * (sector_count + 1);
    const auto next_stack_start_index = stack_start_index + sector_count + 1;

    for (unsigned j = 0; j < sector_count; j++) {
      mesh.indices.push_back(stack_start_index + j);
      mesh.indices.push_back(next_stack_start_index + j + 1);
      mesh.indices.push_back(next_stack_start_index + j);

      mesh.indices.push_back(stack_start_index + j);
      mesh.indices.push_back(stack_start_index + j + 1);
      mesh.indices.push_back(next_stack_start_index + j + 1);
    }
  }

  return mesh;
}
