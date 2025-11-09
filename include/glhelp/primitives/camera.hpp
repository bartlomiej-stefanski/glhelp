#pragma once

#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <glhelp/primitives/PositionProvider.hpp>
#include <glhelp/window.hpp>

namespace glhelp {

template< PositionProvider PositionSource >
class Camera : public PositionSource {
public:
  Camera(std::shared_ptr< Window > window, PositionSource position_source, float fov, float near_clip, float far_clip)
      : PositionSource(position_source), fov(fov), near_clip(near_clip), far_clip(far_clip), window(window)
  {
  }

  void replace_window(std::shared_ptr< Window > new_window)
  {
    window = new_window;
  }

  float get_fov() const { return fov; }
  void set_fov(float new_fov) { fov = new_fov; }

  glm::mat4 get_view_matrix() const
  {
    /* This is essentially the inverse of 'model transformation matrix.
     * M = T * R;
     * V = (T * R)^-1 = R^-1  *  T^-1
     * The logic behind is that we need to 'translate the world' and not ourselves.
     */

    std::cout << "position: " << this->get_position().x << " " << this->get_position().y << " " << this->get_position().z << "\n";
    glm::vec3 euler_angles{glm::eulerAngles(this->get_rotation())};
    std::cout << "rotation: " << glm::degrees(euler_angles.x) << " " << glm::degrees(euler_angles.y) << " " << glm::degrees(euler_angles.z) << "\n";

    const glm::quat inv_rotation{glm::inverse(this->get_rotation())};
    const glm::mat4 inv_rotation_mat{glm::mat4_cast(inv_rotation)};
    const glm::mat4 inv_translation_mat{glm::translate(glm::mat4{1.0}, -this->get_position())};

    return inv_rotation_mat * inv_translation_mat;
  }

  glm::mat4 get_projection_matrix() const
  {
    return glm::perspective(glm::radians(fov), window->aspect_ratio(), near_clip, far_clip);
  }

private:
  float fov;
  float near_clip, far_clip;

  std::shared_ptr< Window > window;

  glm::mat4 perspective_matrix;
};

} // namespace glhelp
