#pragma once

#include <exception>
#include <functional>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_set>
#include <vector>

#include <glm/glm.hpp>

#include <glhelp/Basic.hpp>
#include <glhelp/MtlShader.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Vertex.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/mesh/Renderable.hpp>

namespace glhelp {

struct ObjParseException : std::exception {
  ObjParseException(const std::string& cause, unsigned line_number);

  template< typename T >
  static auto raise(const std::string& cause, unsigned line_number) -> std::function< T() >
  {
    return [cause, line_number]() {
      throw ObjParseException(cause, line_number);
      return T{};
    };
  }

  [[nodiscard]] auto what() const noexcept -> const char* override;

private:
  std::string message;
};

/// Represents inner .obj parser state.
struct ParseState {
  /// Indices pointing to attributes of each vertex used in the object.
  struct VertexData {
    unsigned vertex;
    std::optional< unsigned > texture;
    std::optional< unsigned > normal;

    auto operator==(const VertexData& other) const noexcept -> bool
    {
      return vertex == other.vertex && texture == other.texture && normal == other.normal;
    }
  };

  enum LineType : std::uint8_t {
    VERTEX,
    VERTEX_NORMAL,
    VERTEX_TEXTURE,
    FACE,
    LINE,
    OBJECT_NAME,
    GROUP_NAME,
    MTLLIB,
    USEMTL,
    SMOOTH_SHADING,
    COMMENT,
  };

  static auto get_line_type(const std::string& prefix) -> std::optional< LineType >;
  static auto parse_faces(std::stringstream& faces) -> std::vector< std::array< VertexData, 3 > >;

  std::optional< std::string > obj_name;

  /* Vertex positions, normals and tex_coords as defined by .obj file;
   * They preserce the original order and will be indexed into by 'face' elements.
   */

  std::vector< glm::vec3 > positions;
  std::vector< glm::vec3 > normals;
  std::vector< glm::vec2 > tex_coords;


  // Maps material name to a set of vertices.
  std::unordered_map< std::string, std::vector< VertexData > > vertices;

  std::vector< std::string > mtl_files;

  unsigned current_line;
  std::unordered_set< std::string > warnings;

  static constexpr std::string NO_MATERIAL{"NO_MATERIAL!"};
  std::string current_material{NO_MATERIAL};

  template< VertexType Vertex >
  void parse_line(std::stringstream& line);
  void log_warnings(std::ostream& out);
};

/// Represents a Mesh Object.
template< VertexType Vertex >
struct Obj {
public:
  struct MaterialGroupData {
    std::string material_name;
    std::size_t indices_start;
    std::size_t indices_count;
  };

  Obj() = default;
  Obj(std::vector< Vertex > vertices, std::vector< unsigned > indices, std::vector< MaterialGroupData > material_groups = {})
      : indices(std::move(indices)), vertices(std::move(vertices)), material_groups(material_groups)
  {
  }

  static auto parse_from_file(std::istream& in_stream) -> Obj;
  static auto parse_from_file(const fs::path& file_name) -> Obj;

  std::optional< std::string > obj_name;
  std::vector< unsigned > indices;
  std::vector< Vertex > vertices;

  std::vector< MaterialGroupData > material_groups;
  std::vector< std::string > mtl_files;

  std::optional< fs::path > obj_dir;

private:
  static auto create_vertex(ParseState& parse_state, const ParseState::VertexData& vertex_data) -> Vertex;
};

template< PositionProvider PositionSource >
class MeshObject : public Renderable, public PositionSource {
public:
  MeshObject(Obj< VertexTextured >&& obj, const PositionSource& initial_position, std::shared_ptr< ShaderProgram > shader);
  ~MeshObject() = default;

  void draw() override;

  [[nodiscard]] auto get_shader() const -> std::shared_ptr< ShaderProgram > override;
  [[nodiscard]] auto get_id() const noexcept -> std::size_t override;
  [[nodiscard]] auto get_wireframe_override() const noexcept -> bool override { return false; }

private:
  unsigned vao{}, vbo{}, ebo;

  std::vector< unsigned > indices;
  std::vector< VertexTextured > vertices;

  GLenum mode{GL_TRIANGLES};

  struct MaterialGroupData {
    std::size_t indices_start;
    std::size_t indices_count;
    MtlMaterial material;

    void set_uniforms(ShaderProgram& shader) const;
  };

  unsigned layout_param_count{0};
  std::vector< MaterialGroupData > material_groups;

  std::shared_ptr< ShaderProgram > shader;
};

} // namespace glhelp

// Include template implementation
#include <glhelp/Obj_impl.hpp>
