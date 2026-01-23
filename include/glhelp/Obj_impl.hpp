#pragma once

// 'classic' header-guard to avoid recursive definition
#ifndef REC_OBJ_GUARD
#define REC_OBJ_GUARD

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <glhelp/Obj.hpp>
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
    auto& vertex_buffer{vertices[current_material]};
    for (const auto& face : faces) {
      for (const auto& vertex : face) {
        vertex_buffer.emplace_back(vertex);
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
auto Obj< Vertex >::create_vertex(const ParseState& parse_state, const ParseState::VertexData& vertex_data) -> Vertex
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
    if (!vertex_data.texture.has_value()) {
      throw std::invalid_argument("VertexTextured vertex type requires texture coordinate data.");
    }
    v.tex_coords = parse_state.tex_coords[vertex_data.texture.value() - 1];
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
        .indices_start = obj.vertices.size(),
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
  return obj;
}

} // namespace glhelp

#endif
