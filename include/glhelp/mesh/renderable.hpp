#pragma once

#include <cstddef>
#include <memory>

#include <glhelp/shader.hpp>

namespace glhelp {

class Renderable {
public:
  virtual void draw() = 0;
  [[nodiscard]] virtual auto get_shader() const -> std::shared_ptr< ShaderProgram > = 0;
  [[nodiscard]] virtual auto get_id() const noexcept -> std::size_t = 0;
  virtual auto operator==(const Renderable& other) -> bool
  {
    return get_id() == other.get_id();
  }
};

} // namespace glhelp

template<>
struct std::hash< glhelp::Renderable > {
  auto operator()(const glhelp::Renderable& renderable) const noexcept -> std::size_t
  {
    return renderable.get_id();
  }
};
