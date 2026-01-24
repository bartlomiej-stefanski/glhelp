// TODO: Add error handling and correctness-checking.

#include "glhelp/Shader.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

#include <glhelp/MtlShader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace glhelp {

static void comment_parse(std::stringstream& line [[maybe_unused]]) {}

MtlTexture::MtlTexture(const std::string& path_str)
{
  int width, height, nrComponents;
  unsigned char* data = stbi_load(path_str.c_str(), &width, &height, &nrComponents, 4);

  if (data == nullptr) {
    throw std::runtime_error(std::format("Failed to read images {}!", path_str));
  }

  std::cerr << std::format("Reading a real texture {}\n", path_str);

  glGenTextures(1, &texture_id);

  stbi_set_flip_vertically_on_load(true);

  const GLenum format{GL_RGBA};

  glBindTexture(GL_TEXTURE_2D, texture_id);

  // Przesłanie obrazka
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

  // Generowanie mipmap (ważne dla wydajności i jakości oddalonych obiektów)
  glGenerateMipmap(GL_TEXTURE_2D);

  // Parametry: Powtarzanie tekstury (dla modeli 3D to zazwyczaj pożądane)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Filtrowanie:
  // Min_Filter: Użyj mipmap liniowo (najładniejszy efekt przy oddalaniu)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  // Mag_Filter: Gdy tekstura jest bardzo blisko (powiększona), użyj interpolacji liniowej
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
}

auto MtlTexture::create_from_path(std::stringstream& line, const fs::path& base_path) -> std::shared_ptr< MtlTexture >
{
  std::string file_name;
  line >> file_name;

  const fs::path texture_path{base_path / file_name};
  const std::string path_str{texture_path.string()};

  if (!fs::exists(texture_path)) {
    std::cerr << std::format("Could not find texture '{}'! Using mock texture.\n", path_str);
    return get_mock_texture();
  }

  if (!memorized_textures.contains(path_str)) {
    memorized_textures[path_str] = std::make_shared< MtlTexture >(MtlTexture(path_str));
  }

  return memorized_textures.at(path_str);
}

auto MtlTexture::get_mock_texture(std::uint8_t r, std::uint8_t g, std::uint8_t b) -> std::shared_ptr< MtlTexture >
{
  static std::optional< std::shared_ptr< MtlTexture > > texture{};

  if (!texture.has_value()) {
    texture.emplace(std::make_shared< MtlTexture >(MtlTexture()));

    glGenTextures(1, &texture.value()->texture_id);
    glBindTexture(GL_TEXTURE_2D, texture.value()->texture_id);

    std::array< std::uint8_t, 3 > data{r, g, b};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  return texture.value();
}

auto MtlMaterial::from_file(const fs::path& mtl_path) -> std::unordered_map< std::string, MtlMaterial >
{
  if (!fs::exists(mtl_path)) {
    throw std::runtime_error(std::format("Shader MTL file {} not found.", mtl_path.string()));
  }

  std::ifstream mtl_file(mtl_path);
  if (!mtl_file.is_open()) {
    throw std::runtime_error(std::format("Could not open shader MTL file {}.", mtl_path.string()));
  }

  const fs::path base_path{mtl_path.parent_path()};
  std::cerr << std::format("Reading materials from: {}\n", base_path.string());

  std::unordered_map< std::string, MtlMaterial > materials;
  std::string current_material_name{};
  auto current_material{[&]() -> MtlMaterial& {return materials.at(current_material_name);}};

  std::unordered_map< std::string, std::function< void(std::stringstream&) > > line_parse{
      // Comments.
      {"#", comment_parse},
      // Create new material.
      {"newmtl", [&](std::stringstream& line) {
         std::string name;
         line >> name;

         std::cerr << std::format("Found a material: {}\n", name);

         materials.emplace(name, MtlMaterial{});
        current_material_name = name;
       }},
      // Read Ambient.
      {"Ka", [&](std::stringstream& line) {
         MtlMaterial& mat{current_material()};
         line >> mat.ambient.x >> mat.ambient.y >> mat.ambient.z;
       }},
      {"map_Ka", [&](std::stringstream& line) {
         current_material().texture_ambient = MtlTexture::create_from_path(line, base_path);
       }},
      // Read Diffuse.
      {"Kd", [&](std::stringstream& line) {
         MtlMaterial& mat{current_material()};
         line >> mat.diffuse.x >> mat.diffuse.y >> mat.diffuse.z;
       }},
      {"map_Kd", [&](std::stringstream& line) {
         current_material().texture_diffuse = MtlTexture::create_from_path(line, base_path);
         // Heuristic! Load diffuse texture as ambient texture when not specified.
         if (current_material().texture_ambient == MtlTexture::get_mock_texture()) {
            current_material().texture_ambient = current_material().texture_diffuse;
         }
       }},
      // Read Specular.
      {"Ks", [&](std::stringstream& line) {
         MtlMaterial& mat{current_material()};
         line >> mat.specular.x >> mat.specular.y >> mat.specular.z;
       }},
      {"map_Ks", [&](std::stringstream& line) {
         current_material().texture_specular = MtlTexture::create_from_path(line, base_path);
       }},
      // Read Specular Exponent.
      {"Ns", [&](std::stringstream& line) {
         MtlMaterial& mat{current_material()};
         line >> mat.shininnes;
       }},
      {"map_Ns", [&](std::stringstream& line) {
         current_material().texture_shininnes = MtlTexture::create_from_path(line, base_path);
       }},
      // Read Transparency.
      {"Tr", [&](std::stringstream& line) {
         MtlMaterial& mat{current_material()};
         line >> mat.translucency;
       }},
      {"d", [&](std::stringstream& line) {
         MtlMaterial& mat{current_material()};
         line >> mat.translucency;
       }},
      // Read Bump Map.
      {"map_bump", [&](std::stringstream& line) {
         current_material().texture_bump = MtlTexture::create_from_path(line, base_path);
       }},
      {"bump", [&](std::stringstream& line) {
         current_material().texture_bump = MtlTexture::create_from_path(line, base_path);
       }},
      // Read Displacement Map.
      {"disp", [&](std::stringstream& line) {
         current_material().texture_displacement = MtlTexture::create_from_path(line, base_path);
       }},
  };

  std::string line;
  while (std::getline(mtl_file, line)) {
    if (line.empty()) {
      continue;
    }

    std::stringstream curr_line(line);
    std::string prefix;
    curr_line >> prefix;
    if (!line_parse.contains(prefix)) {
      std::cerr << std::format("Unknown prefix in .mtl file: '{}'; ignoring...\n", prefix);
    }
    else {
      line_parse.at(prefix)(curr_line);
    }
  }

  return materials;
}

} // namespace glhelp
