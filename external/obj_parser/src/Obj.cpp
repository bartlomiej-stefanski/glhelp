
#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <obj_parser/Obj.hpp>

namespace obj_parser
{

ObjParseException::ObjParseException(const std::string& cause, unsigned line_number)
  : message("ObjParseException at line " + std::to_string(line_number) + ": " + cause)
{
}

auto ObjParseException::what() const noexcept -> const char*
{
  return message.c_str();
}

auto get_line_type(const std::string& prefix) -> std::optional< LineType >
{
  if (prefix == "v") {
    return VERTEX;
  }
  if (prefix == "f") {
    return FACE;
  }
  if (prefix == "o") {
    return OBJECT_NAME;
  }
  if (prefix == "g") {
    return GROUP_NAME;
  }
  if (prefix == "#") {
    return COMMENT;
  }

  return std::nullopt;
}


auto parse_faces(std::stringstream& faces) -> std::array< ParseState::VertexData, 3 >
{
  /* Possible formats:
   * - v
   * - v/vt
   * - v/vt/vn
   * - v//vn
   */
  std::array< ParseState::VertexData, 3 > vertices;
  for (auto& vertex: vertices) {
    faces >> vertices[0].vertex; // Read v
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
  }

  return vertices;
}

}
