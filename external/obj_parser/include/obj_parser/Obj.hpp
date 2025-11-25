#pragma once

#include <cstdint>
#include <exception>
#include <functional>
#include <istream>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <obj_parser/Vertex.hpp>

namespace obj_parser
{

class ObjParseException : std::exception
{
private:
  ObjParseException(const std::string& cause, unsigned line_number);

public:
  template< typename T >
  static std::function< T() > raise(const std::string& cause, unsigned line_number)
  {
    return [cause, line_number]() {
      throw ObjParseException(cause, line_number);
      return T{};
    };
  }

  const char* what() const noexcept override;

private:
  std::string message;
};

/// Represents inner .obj parser state.
struct ParseState
{
  unsigned current_line;
  std::vector< glm::vec3 > positions;
  std::vector< glm::vec3 > normals;
  std::vector< glm::vec2 > tex_coords;

  struct VertexData {
    unsigned vertex;
    std::optional< unsigned > texture;
    std::optional< unsigned > normal;

    auto operator==(const VertexData& other) const noexcept -> bool
    {
      return vertex == other.vertex
        && texture == other.texture
        && normal == other.normal;
    }
  };

  std::vector< VertexData > vertices;
  std::vector< unsigned > indices;

  std::optional< std::string > obj_name;
};

/// Represents a Mesh Object.
template< VertexType Vertex >
struct Obj
{
public:
  Obj() = default;

  static auto parse_from_file(std::istream& in_stream) -> Obj;
  static auto parse_from_file(const std::string& file_name) -> Obj;

  std::optional< std::string > obj_name;
  std::vector< unsigned > indices;
  std::vector< Vertex > vertices;

private:
  static auto create_vertex(const ParseState& parse_state, const ParseState::VertexData& vertex_data) -> Vertex;
  static void parse_line(std::stringstream& line, ParseState& parse_state);
};

}

// Include template implementation
#include <obj_parser/Obj_impl.hpp>
