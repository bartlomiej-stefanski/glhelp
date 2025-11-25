#pragma once

// 'classic' header-guard to avoid recursive definition
#ifndef REC_OBJ_GUARD
#define REC_OBJ_GUARD

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <obj_parser/Vertex.hpp>
#include <obj_parser/Obj.hpp>

template<>
struct std::hash< obj_parser::ParseState::VertexData >
{
  auto operator()(const obj_parser::ParseState::VertexData& vertex) const noexcept -> std::size_t
  {
    return vertex.vertex * vertex.normal.value_or(2) * vertex.texture.value_or(17);
  }
};

namespace obj_parser
{

enum LineType : std::uint8_t {
  VERTEX,
  VERTEX_NORMAL,
  VERTEX_TEXTURE,
  FACE,
  OBJECT_NAME,
  GROUP_NAME,
  COMMENT,
};

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
auto Obj< Vertex >::parse_from_file(const std::string& file_name) -> Obj< Vertex >
{
  std::ifstream file_stream(file_name);

  if (!file_stream.is_open()) {
    throw std::invalid_argument("Could not find '" + file_name + "'.");
  }

  return parse_from_file(file_stream);
}

template< VertexType Vertex >
auto Obj< Vertex >::parse_from_file(std::istream& in_stream) -> Obj< Vertex >
{
  ParseState parse_state{};

  std::string line;
  while (std::getline(in_stream, line)) {
    parse_state.current_line++;
    std::stringstream ss(std::move(line));

    if (line.empty()) {
      continue;
    }

    parse_line(ss, parse_state);
  }

  Obj< Vertex > obj;
  std::unordered_map< ParseState::VertexData, std::size_t > deduplicator;
  for (const auto& vertex: parse_state.vertices) {
    if (!deduplicator.contains(vertex)) {
      obj.vertices.emplace_back(Obj< Vertex >::create_vertex(parse_state, vertex));
      deduplicator[vertex] = obj.vertices.size() - 1;
    }
  }

  for (const auto& index: parse_state.indices) {
    const auto& curr_data{parse_state.vertices[index-1]};
    obj.indices.emplace_back(deduplicator.at(curr_data));
  }

  obj.obj_name = std::move(parse_state.obj_name);
  return obj;
}

auto get_line_type(const std::string& prefix) -> std::optional< LineType >;
auto parse_faces(std::stringstream& faces) -> std::array< ParseState::VertexData, 3 >;

template< VertexType Vertex >
void Obj< Vertex >::parse_line(std::stringstream& line, ParseState& parse_state)
{
  std::string prefix{};
  line >> prefix;
  const auto line_type{get_line_type(prefix).or_else(
    ObjParseException::raise< std::optional< LineType > >("unknown prefix", parse_state.current_line)
  ).value()};

  switch (line_type) {
    case VERTEX: {
      glm::vec3 position{};
      line >> position.x >> position.y >> position.z;
      parse_state.positions.emplace_back(position);
      break;
    }
    case VERTEX_NORMAL: {
      if constexpr (!VertexWithNormal< Vertex >) {
        std::cerr << "Encountered Vertex-Normal data when parsing into object without such data\n";
      }
      else {
        glm::vec3 normal{};
        line >> normal.x >> normal.y >> normal.z;
        parse_state.normals.emplace_back(normal);
      }
      break;
    }
    case VERTEX_TEXTURE: {
      if constexpr (!VertexWithTexture< Vertex >) {
        std::cerr << "Encountered Vertex-Texture data when parsing into object without such data\n";
      }
      else {
        glm::vec3 texture{};
        line >> texture.x >> texture.y >> texture.z;
        parse_state.tex_coords.emplace_back(texture);
      }
      break;
    }
    case FACE: {
      auto face_vertices{parse_faces(line)};
      parse_state.vertices.emplace_back(face_vertices[0]);
      parse_state.vertices.emplace_back(face_vertices[1]);
      parse_state.vertices.emplace_back(face_vertices[2]);
      break;
    }
    case OBJECT_NAME: {
      std::string name{};
      line >> name;
      parse_state.obj_name = name;
      break;
    }
    case GROUP_NAME:
    case COMMENT:
      // ignore group names and comments
      break;
  }
}

}

#endif
