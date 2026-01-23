#pragma once

#include <array>
#include <filesystem>
#include <unordered_map>

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <glhelp/Basic.hpp>

namespace glhelp {

struct Material {
  glm::vec3 ambient{1.0F};
  glm::vec3 diffuse{1.0F};
  glm::vec3 specular{1.0F};

  float shininnes{1.0F};

  float translucency{1.0F};

  union Textures {
    std::array< GLuint, 6 > direct;
    struct {
      GLuint ambient{0};
      GLuint diffuse{0};
      GLuint specular{0};
      GLuint shininnes{0};
      GLuint bump{0};
      GLuint displacement{0};
    };
  } textures;

  static auto from_file(const fs::path& mtl_path) -> std::unordered_map< std::string, Material >;

private:
  std::optional< GLuint > blank_texture;
};

} // namespace glhelp
