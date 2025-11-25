
#include <array>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>

#include <obj_parser/Obj.hpp>
#include <unordered_map>

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

void ParseState::log_warnings(std::ostream& out) {
  if (warnings.empty()) {
    return;
  }

  out << "OBJ parser recorded " << warnings.size() << " warning(s) for object '" << (obj_name.has_value() ? *obj_name : "<unnamed>") << "':\n";
  for (const auto& warning: warnings) {
    out << "- " << warning << "\n";
  }
}

auto get_line_type(const std::string& prefix) -> std::optional< LineType >
{
  static const std::unordered_map< std::string, LineType> prefix_map{
    {"v", VERTEX},
    {"vt", VERTEX_TEXTURE},
    {"vn", VERTEX_NORMAL},
    {"f", FACE},
    {"o", OBJECT_NAME},
    {"g", GROUP_NAME},
    {"s", SMOOTH_SHADING},
    {"#", COMMENT},
  };

  if (prefix_map.contains(prefix)) {
    return prefix_map.at(prefix);
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
  }

  return vertices;
}

}
