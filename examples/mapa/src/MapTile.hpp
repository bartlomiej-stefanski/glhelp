#pragma once

#include <memory>

#include <glad/gl.h>

#include <glhelp/Obj.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/position/Position.hpp>

#include "TerrainMap.hpp"

/// Represents a layer of a tiled map.
class MapTile : public glhelp::InstancedMesh3d< glhelp::CachingSimplePosition, glm::vec3, float > {
public:
  MapTile(
      std::shared_ptr< TerrainMap > terrain_map,
      const glhelp::Obj< glhelp::SimpleVertex >& plane,
      std::shared_ptr< glhelp::ShaderProgram > shader_program,
      u8 layer);

  const u8 layer;

  [[nodiscard]] auto get_wireframe_override() const noexcept -> bool override { return true; };

  void update_texture_layers(TerrainMapArea map_area);

  static void set_draw_boundaries(glm::vec2 from, glm::vec2 to)
  {
    draw_from = from;
    draw_to = to;
  }

private:
  void uniform_setter_callback() override;

  std::shared_ptr< TerrainMap > terrain_map;

  std::vector< float > texture_layer_indices;

  inline static glm::vec2 draw_from;
  inline static glm::vec2 draw_to;
};
