#pragma once

#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <glhelp/primitives/PositionProvider.hpp>
#include <glhelp/window.hpp>

namespace glhelp {

template< PositionProvider PositionSource >
class Camera {
public:
  Camera(std::shared_ptr< Window > window, PositionSource position_source, float fov, float near_clip, float far_clip)
      : position_source(position_source), fov(fov), near_clip(near_clip), far_clip(far_clip), window(window)
  {
  }

  void replace_window(std::shared_ptr< Window > new_window)
  {
    window = new_window;
  }

  glm::mat4 get_view_matrix() const
  {
    /* This is essentially the inverse of 'model transformation matrix.
     * M = T * R;
     * V = (T * R)^-1 = R^-1  *  T^-1
     * The logic behind is that we need to 'translate the world' and not ourselves.
     */
    const glm::quat inv_rotation{
        glm::angleAxis(-position_source.get_yaw(), UP_VECTOR) * glm::angleAxis(-position_source.get_pitch(), RIGHT_VECTOR) * glm::angleAxis(-position_source.get_roll(), FORWARD_VECTOR)};
    const glm::mat4 rotation_inv{glm::mat4_cast(inv_rotation)};
    const glm::mat4 translate_inv{glm::translate(glm::mat4{1.0}, -position_source.get_position())};

    return rotation_inv * translate_inv;
  }

  glm::mat4 get_projection_matrix()
  {
    return glm::perspectiveFov(fov, window->aspect_ratio(), 1.0f, near_clip, far_clip);
  }

private:
  PositionSource position_source;
  float fov;
  float near_clip, far_clip;

  std::shared_ptr< Window > window;
};

} // namespace glhelp
