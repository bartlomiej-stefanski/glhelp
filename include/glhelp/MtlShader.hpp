#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <glhelp/Basic.hpp>
#include <glhelp/Texture.hpp>

namespace glhelp {

class MtlTexture : public Texture< GL_TEXTURE_2D > {
  MtlTexture() = default;
  MtlTexture(const std::string& path_str);

public:
  static auto create_from_path(std::stringstream& line, const fs::path& base_path) -> std::shared_ptr< MtlTexture >;
  static auto get_mock_texture(std::uint8_t r = 255, std::uint8_t g = 255, std::uint8_t b = 255) -> std::shared_ptr< MtlTexture >;

  bool has_alpha{0};
  int channels{1};

private:
  inline static std::unordered_map< std::string, std::shared_ptr< MtlTexture > > memorized_textures{};
};

struct MtlMaterial {
  MtlMaterial() = default;

  glm::vec3 ambient{1.0F};
  glm::vec3 diffuse{1.0F};
  glm::vec3 specular{1.0F};

  float shininnes{1.0F};

  float translucency{1.0F};

  std::shared_ptr< MtlTexture > texture_ambient{MtlTexture::get_mock_texture()};
  std::shared_ptr< MtlTexture > texture_diffuse{MtlTexture::get_mock_texture()};
  std::shared_ptr< MtlTexture > texture_specular{MtlTexture::get_mock_texture()};
  std::shared_ptr< MtlTexture > texture_shininnes{MtlTexture::get_mock_texture()};
  std::shared_ptr< MtlTexture > texture_bump{MtlTexture::get_mock_texture()};
  std::shared_ptr< MtlTexture > texture_displacement{MtlTexture::get_mock_texture()};
  std::shared_ptr< MtlTexture > texture_transparent{MtlTexture::get_mock_texture()};
  int transparency_layer{1};

  static auto from_file(const fs::path& mtl_path) -> std::unordered_map< std::string, MtlMaterial >;

private:
  std::optional< GLuint > blank_texture;
};

} // namespace glhelp
