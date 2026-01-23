#include "glhelp/Shader.hpp"
#include "glhelp/Vertex.hpp"
#include <array>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <glhelp/Obj.hpp>

namespace glhelp {

ObjParseException::ObjParseException(const std::string& cause, unsigned line_number)
    : message("ObjParseException at line " + std::to_string(line_number) + ": " + cause)
{
}

auto ObjParseException::what() const noexcept -> const char*
{
  return message.c_str();
}

void ParseState::log_warnings(std::ostream& out)
{
  if (warnings.empty()) {
    return;
  }

  out << "OBJ parser recorded " << warnings.size() << " warning(s) for object '" << (obj_name.has_value() ? *obj_name : "<unnamed>") << "':\n";
  for (const auto& warning : warnings) {
    out << "- " << warning << "\n";
  }
}

auto ParseState::get_line_type(const std::string& prefix) -> std::optional< LineType >
{
  static const std::unordered_map< std::string, LineType > prefix_map{
      {"v", VERTEX},
      {"vt", VERTEX_TEXTURE},
      {"vn", VERTEX_NORMAL},
      {"f", FACE},
      {"l", LINE},
      {"o", OBJECT_NAME},
      {"g", GROUP_NAME},
      {"mtllib", MTLLIB},
      {"usemtl", USEMTL},
      {"s", SMOOTH_SHADING},
      {"#", COMMENT},
  };

  if (prefix_map.contains(prefix)) {
    return prefix_map.at(prefix);
  }

  return std::nullopt;
}

auto ParseState::parse_faces(std::stringstream& faces) -> std::vector< std::array< VertexData, 3 > >
{
  /* Possible formats:
   * - v
   * - v/vt
   * - v/vt/vn
   * - v//vn
   */
  std::vector< VertexData > vertices;
  while (!faces.eof()) {
    VertexData vertex;
    faces >> vertex.vertex; // Read v
    if (faces.peek() == '/') {
      faces.get();
      if (faces.peek() != '/') {
        faces >> (vertex.texture.emplace()); // Read vt
      }

      if (faces.peek() == '/') {
        faces.get();
        faces >> (vertex.normal.emplace()); // Read vn
      }
    }
    vertices.emplace_back(vertex);
  }

  if (vertices.size() <= 2) {
    throw std::runtime_error("Too little values for face provided!");
  }

  std::vector< std::array< VertexData, 3 > > triangle_faces;
  for (std::size_t i{2}; i < vertices.size(); i++) {
    triangle_faces.emplace_back(std::array< VertexData, 3 >{
        vertices[i - 2],
        vertices[i - 1],
        vertices[i]});
  }

  return triangle_faces;
}

MeshObject::MeshObject(Obj< VertexTextured >&& obj)
{
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(VertexTextured), obj.vertices.data(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size() * sizeof(GLuint), obj.indices.data(), GL_STATIC_DRAW);

  // Enable Position Attribute.
  glEnableVertexAttribArray(0);
  layout_param_count++;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)offsetof(VertexTextured, position));

  // Enable Normal Attribute.
  glEnableVertexAttribArray(1);
  layout_param_count++;
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)offsetof(VertexTextured, normal));

  // Enable Textured Coordinate Attribute.
  glEnableVertexAttribArray(2);
  layout_param_count++;
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)offsetof(VertexTextured, tex_coords));

  glBindVertexArray(0);
}

void MeshObject::draw()
{
}

auto MeshObject::get_shader() const -> std::shared_ptr< ShaderProgram >
{
  return nullptr;
}

auto MeshObject::get_id() const noexcept -> std::size_t
{
  return vao;
}

} // namespace glhelp
