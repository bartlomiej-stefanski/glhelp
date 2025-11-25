#pragma once

#include <glm/glm.hpp>

namespace obj_parser
{

struct SimpleVertex
{
  glm::vec3 position;
};

struct VertexNormals
{
  glm::vec3 position;
  glm::vec3 normal;
};

struct VertexTextured
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

template< typename T >
concept VertexType = requires(const T& vt) {
  { T{} } -> std::convertible_to< T >;
  { vt.position } -> std::convertible_to< glm::vec3 >;
};

template<typename T>
concept VertexWithNormal = requires(const T& vn) {
  { vn.normal } -> std::convertible_to< glm::vec3 >;
};

template<typename T>
concept VertexWithTexture = requires(const T& vt) {
  { vt.tex_coords } -> std::convertible_to< glm::vec2 >;
};

}
