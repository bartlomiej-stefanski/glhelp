// TODO: Add error handling and correctness-checking.

#include "glhelp/Shader.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <glhelp/MtlShader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace glhelp {

static auto create_mock_texture(std::uint8_t r = 255, std::uint8_t g = 255, std::uint8_t b = 255) -> unsigned int
{
  static unsigned int textureID;
  static bool create_texture{true};

  if (!create_texture) {
    create_texture = false;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    std::array< std::uint8_t, 3 > data{r, g, b};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  return textureID;
}

static void comment_parse(std::stringstream& line [[maybe_unused]]) {}

static void read_texture(std::stringstream& line, const fs::path& base_path, GLuint& texture_id)
{
  glGenTextures(1, &texture_id);

  int width, height, nrComponents;
  stbi_set_flip_vertically_on_load(true);

  std::string file_name;
  line >> file_name;
  fs::path path{base_path / file_name};
  const std::string path_str{path.string()};
  unsigned char* data = stbi_load(path_str.c_str(), &width, &height, &nrComponents, 0);

  if (data == nullptr) {
    throw std::runtime_error(std::format("Failed to read images {}!", path_str));
  }
  else {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;
    else
      throw std::runtime_error(std::format("Unknown format of {}", path_str));

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
}

auto Material::from_file(const fs::path& mtl_path) -> std::unordered_map< std::string, Material >
{
  if (!fs::exists(mtl_path)) {
    throw std::runtime_error(std::format("Shader MTL file {} not found.", mtl_path.string()));
  }

  std::ifstream mtl_file(mtl_path);
  if (!mtl_file.is_open()) {
    throw std::runtime_error(std::format("Could not open shader MTL file {}.", mtl_path.string()));
  }

  const fs::path base_path{mtl_path.parent_path()};

  std::unordered_map< std::string, Material > materials;
  std::optional< std::reference_wrapper< Material > > current_material;

  std::unordered_map< std::string, std::function< void(std::stringstream&) > > line_parse{
      // Comments.
      {"#", comment_parse},
      // Create new material.
      {"newmtl", [&](std::stringstream& line) {
         std::string name;
         line >> name;
         materials.emplace(name, Material{});
         current_material = materials.at(name);
         for (auto& texture : current_material.value().get().textures.direct) {
           texture = create_mock_texture();
         }
       }},
      // Read Ambient.
      {"Ka", [&](std::stringstream& line) {
         Material& mat{current_material.value().get()};
         line >> mat.ambient.x >> mat.ambient.y >> mat.ambient.z;
       }},
      {"map_Ka", [&](std::stringstream& line) {
         read_texture(line, base_path, current_material.value().get().textures.ambient);
       }},
      // Read Diffuse.
      {"Kd", [&](std::stringstream& line) {
         Material& mat{current_material.value().get()};
         line >> mat.diffuse.x >> mat.diffuse.y >> mat.diffuse.z;
       }},
      {"map_Kd", [&](std::stringstream& line) {
         read_texture(line, base_path, current_material.value().get().textures.diffuse);
       }},
      // Read Specular.
      {"Ks", [&](std::stringstream& line) {
         Material& mat{current_material.value().get()};
         line >> mat.specular.x >> mat.specular.y >> mat.specular.z;
       }},
      {"map_Ks", [&](std::stringstream& line) {
         read_texture(line, base_path, current_material.value().get().textures.specular);
       }},
      // Read Specular Exponent.
      {"Ns", [&](std::stringstream& line) {
         Material& mat{current_material.value().get()};
         line >> mat.shininnes;
       }},
      {"map_Ns", [&](std::stringstream& line) {
         read_texture(line, base_path, current_material.value().get().textures.shininnes);
       }},
      // Read Transparency.
      {"Tr", [&](std::stringstream& line) {
         Material& mat{current_material.value().get()};
         line >> mat.translucency;
       }},
      {"d", [&](std::stringstream& line) {
         Material& mat{current_material.value().get()};
         line >> mat.translucency;
       }},
      // Read Bump Map.
      {"map_bump", [&](std::stringstream& line) {
         read_texture(line, base_path, current_material.value().get().textures.bump);
       }},
      {"bump", [&](std::stringstream& line) {
         read_texture(line, base_path, current_material.value().get().textures.bump);
       }},
      // Read Displacement Map.
      {"disp", [&](std::stringstream& line) {
         read_texture(line, base_path, current_material.value().get().textures.displacement);
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
    line_parse.at(prefix)(curr_line);
  }

  return materials;
}

} // namespace glhelp
