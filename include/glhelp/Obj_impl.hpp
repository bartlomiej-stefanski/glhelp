#pragma once

// 'classic' header-guard to avoid recursive definition
#include <chrono>
#include <glm/geometric.hpp>
#ifndef REC_OBJ_GUARD
#define REC_OBJ_GUARD

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <glhelp/MtlShader.hpp>
#include <glhelp/Obj.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Vertex.hpp>

template<>
struct std::hash< glhelp::ParseState::VertexData > {
  auto operator()(const glhelp::ParseState::VertexData& vertex) const noexcept -> std::size_t
  {
    return vertex.vertex * vertex.normal.value_or(2) * vertex.texture.value_or(17);
  }
};

namespace glhelp {

template< VertexType Vertex >
void ParseState::parse_line(std::stringstream& line)
{
  std::string prefix{};
  line >> prefix;

  const auto line_type{get_line_type(prefix).or_else(
                                                ObjParseException::raise< std::optional< LineType > >("unknown prefix", current_line))
                           .value()};

  switch (line_type) {
  case VERTEX: {
    glm::vec3 position{};
    line >> position.x >> position.y >> position.z;

    min_pos.x = glm::min(min_pos.x, position.x);
    min_pos.y = glm::min(min_pos.y, position.y);
    min_pos.z = glm::min(min_pos.z, position.z);

    max_pos.x = glm::max(max_pos.x, position.x);
    max_pos.y = glm::max(max_pos.y, position.y);
    max_pos.z = glm::max(max_pos.z, position.z);

    positions.emplace_back(position);
    break;
  }
  case VERTEX_NORMAL: {
    if constexpr (!VertexWithNormal< Vertex >) {
      warnings.insert("Encountered Vertex-Normal data when parsing into object without such data.\n");
    }
    else {
      glm::vec3 normal{};
      line >> normal.x >> normal.y >> normal.z;
      normals.emplace_back(normal);
    }
    break;
  }
  case VERTEX_TEXTURE: {
    if constexpr (!VertexWithTexture< Vertex >) {
      warnings.insert("Encountered Vertex-Texture data when parsing into object without such data.\n");
    }
    else {
      glm::vec3 texture{};
      line >> texture.x >> texture.y >> texture.z;
      tex_coords.emplace_back(texture);
    }
    break;
  }
  case FACE: {
    auto faces{parse_faces(line)};
    for (const auto& face : faces) {
      for (const auto& vertex : face) {
        vertices[current_material].emplace_back(vertex);
      }
    }
    break;
  }
  case OBJECT_NAME: {
    std::string name{};
    line >> name;
    obj_name = std::move(name);
    break;
  }
  case MTLLIB: {
    std::string mtl_file;
    line >> mtl_file;
    mtl_files.emplace_back(std::move(mtl_file));
    break;
  }
  case USEMTL: {
    line >> current_material;
    break;
  }
  case GROUP_NAME:
  case COMMENT:
  case SMOOTH_SHADING:
  case LINE:
    // TODO: Handle these cases.
    break;
  }
}

template< VertexType Vertex >
auto Obj< Vertex >::create_vertex(ParseState& parse_state, const ParseState::VertexData& vertex_data) -> Vertex
{
  Vertex v{};
  v.position = parse_state.positions[vertex_data.vertex - 1];

  if constexpr (VertexWithNormal< Vertex >) {
    if (!vertex_data.normal.has_value()) {
      throw std::invalid_argument("VertexNormals vertex type requires normal data.");
    }
    v.normal = parse_state.normals[vertex_data.normal.value() - 1];
  }

  if constexpr (VertexWithTexture< Vertex >) {
    parse_state.warnings.insert("While parsing for type VertexWithTexture type encountered vertices without Texture data! using default...");
    if (!vertex_data.texture.has_value()) {
      v.tex_coords = glm::vec2{0.0F};
    }
    else {
      v.tex_coords = parse_state.tex_coords[vertex_data.texture.value() - 1];
    }
  }

  return v;
}

template< VertexType Vertex >
auto Obj< Vertex >::parse_from_file(const fs::path& file_path) -> Obj< Vertex >
{
  if (!fs::exists(file_path)) {
    throw std::runtime_error(std::format("File path: {} does not exist!", file_path.string()));
  }

  std::ifstream file_stream(file_path);

  if (!file_stream.is_open()) {
    throw std::invalid_argument(std::format("Could not open '{}'.", file_path.string()));
  }

  auto obj{parse_from_file(file_stream)};
  obj.obj_dir = file_path.parent_path(); // File must have a parent-path.
  file_stream.close();
  return obj;
}

template< VertexType Vertex >
auto Obj< Vertex >::parse_from_file(std::istream& in_stream) -> Obj< Vertex >
{
  ParseState parse_state{};

  std::string line;
  while (std::getline(in_stream, line)) {
    parse_state.current_line++;
    if (line.empty()) {
      continue;
    }

    std::stringstream ss(std::move(line));
    parse_state.parse_line< Vertex >(ss);
  }

  Obj< Vertex > obj;
  obj.material_groups.reserve(parse_state.vertices.size());

  std::unordered_map< ParseState::VertexData, std::size_t > deduplicator;
  for (const auto& [material, vertices] : parse_state.vertices) {
    obj.material_groups.emplace_back(MaterialGroupData{
        .material_name = material,
        .indices_start = obj.indices.size(),
        .indices_count = vertices.size()});

    for (const auto& vertex : vertices) {
      if (!deduplicator.contains(vertex)) {
        obj.vertices.emplace_back(Obj< Vertex >::create_vertex(parse_state, vertex));
        deduplicator.emplace(vertex, obj.vertices.size() - 1);
      }

      obj.indices.emplace_back(deduplicator.at(vertex));
    }
  }

  parse_state.log_warnings(std::cerr);

  obj.obj_name = std::move(parse_state.obj_name);
  obj.mtl_files = std::move(parse_state.mtl_files);
  obj.radius = glm::length(parse_state.min_pos) + glm::length(parse_state.max_pos);
  return obj;
}

template< PositionProvider PositionSource >
MeshObject< PositionSource >::MeshObject(Obj< VertexTextured >&& obj, const PositionSource& initial_position, std::shared_ptr< ShaderProgram > shader)
  : PositionSource(initial_position), radius(obj.radius), indices(std::move(obj.indices)), vertices(std::move(obj.vertices)), shader(std::move(shader))
{
  if (!obj.obj_dir.has_value()) {
    throw std::runtime_error("Cannot create a MeshObject without 'path' present in Obj!");
  }

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexTextured), vertices.data(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  // Enable Position Attribute.
  glEnableVertexAttribArray(0);
  layout_param_count++;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), reinterpret_cast< void* >(offsetof(VertexTextured, position)));

  // Enable Normal Attribute.
  glEnableVertexAttribArray(1);
  layout_param_count++;
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), reinterpret_cast< void* >(offsetof(VertexTextured, normal)));

  // Enable Textured Coordinate Attribute.
  glEnableVertexAttribArray(2);
  layout_param_count++;
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), reinterpret_cast< void* >(offsetof(VertexTextured, tex_coords)));

  glBindVertexArray(0);

  const fs::path base_path{obj.obj_dir.value()};
  std::unordered_map< std::string, MtlMaterial > materials;
  for (const auto& mtl_file: obj.mtl_files) {
    fs::path mtl_path{base_path / mtl_file};
    std::cerr << "Reading materials from file: " << mtl_path.string() << "\n";
    auto file_materials{MtlMaterial::from_file(mtl_path)};
    for (auto& [name, mat]: file_materials) {
      if (materials.contains(name)) {
        throw std::runtime_error("Conflicting material names found!");
      }

      std::cerr << "Found material: " << name << "\n";
      materials.emplace(name, std::move(mat));
    }
  }

  for (const auto& material: obj.material_groups) {
    if (material.material_name == ParseState::NO_MATERIAL || !materials.contains(material.material_name)) {
      std::cerr << "Using a 'default' material\n";
      material_groups.emplace_back(MaterialGroupData{
        .indices_start = material.indices_start,
        .indices_count = material.indices_count,
        .material = MtlMaterial{}
      });
    }
    else {
      material_groups.emplace_back(MaterialGroupData{
        .indices_start = material.indices_start,
        .indices_count = material.indices_count,
        .material = materials.at(material.material_name)
      });
    }
  }
}

template< PositionProvider PositionSource >
void MeshObject< PositionSource >::draw()
{
  glBindVertexArray(vao);

  const auto model_matrix{get_model_matrix(*this)};
  shader->set_uniform("uModelTransform", model_matrix);

  const auto normal_transform{glm::mat3{glm::transpose(glm::inverse(model_matrix))}};
  shader->set_uniform("uNormalTransform", normal_transform);

  for (const auto& material : material_groups) {
    material.set_uniforms(*shader);
    glDrawElements(
      GL_TRIANGLES,
      material.indices_count,
      GL_UNSIGNED_INT,
      reinterpret_cast< void* >(material.indices_start * sizeof(GLuint)));
  }

  glBindVertexArray(0);
}

template< PositionProvider PositionSource >
auto MeshObject< PositionSource >::get_shader() const -> std::shared_ptr< ShaderProgram >
{
  return shader;
}

template< PositionProvider PositionSource >
auto MeshObject< PositionSource >::get_id() const noexcept -> std::size_t
{
  return vao;
}

template< PositionProvider PositionSource >
void MeshObject< PositionSource >::MaterialGroupData::set_uniforms(ShaderProgram& shader [[maybe_unused]]) const
{
  shader.set_uniform("uAmbient", material.ambient);
  shader.set_uniform("uDiffuse", material.diffuse);
  shader.set_uniform("uSpecular", material.specular);
  shader.set_uniform("uShininnes", material.shininnes);
  shader.set_uniform("uTranslucency", material.translucency);

  material.texture_ambient->load_to_texture_unit(0);
  shader.set_uniform< Texture< GL_TEXTURE_2D > >("mapAmbient", *(material.texture_ambient));

  material.texture_diffuse->load_to_texture_unit(1);
  shader.set_uniform< Texture< GL_TEXTURE_2D > >("mapDiffuse", *(material.texture_diffuse));

  material.texture_specular->load_to_texture_unit(2);
  shader.set_uniform< Texture< GL_TEXTURE_2D > >("mapSpecular", *(material.texture_specular));

  material.texture_transparent->load_to_texture_unit(3);
  shader.set_uniform< Texture< GL_TEXTURE_2D > >("mapTranslucency", *(material.texture_transparent));
}

} // namespace glhelp

#endif
