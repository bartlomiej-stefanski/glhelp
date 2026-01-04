#pragma once

#include <memory>

#include <glad/gl.h>

#include <glhelp/mesh/InstancedMesh3D.hpp>
#include <glhelp/position/Position.hpp>

#include <obj_parser/Obj.hpp>

#include <hgt/Hgt.hpp>

#include "TerrainMap.hpp"

/// Represents a layer of a tiled map.
class MapTile3d : public glhelp::InstancedMesh3d< glhelp::CachingSimplePosition, glm::vec3, float > {
public:
  MapTile3d(
      std::shared_ptr< TerrainMap > terrain_map,
      const obj_parser::Obj< obj_parser::SimpleVertex >& plane,
      std::shared_ptr< glhelp::ShaderProgram > shader_program,
      u8 texture_layer,
      float tile_scale);

  MapTile3d(
      std::shared_ptr< TerrainMap > terrain_map,
      const obj_parser::Obj< obj_parser::SimpleVertex >& plane,
      std::shared_ptr< glhelp::ShaderProgram > shader_program,
      u8 texture_layer,
      float tile_scale,
      unsigned tiles_edge);

  const float tile_scale;
  const u8 texture_layer;

  [[nodiscard]] auto get_wireframe_override() const noexcept -> bool override { return true; };

  auto move_to_lat_lon(glm::vec2 lat_lon) -> TerrainMapArea;
  void update_texture_layers();

  inline static glm::vec2 camera_lat_lon;
  inline static float heigth_scale{1.0F};
  inline static float lod_tess{1.0F};

  [[nodiscard]] auto is_hq() const noexcept -> bool { return edge_count.has_value(); }

  static void set_draw_boundaries(glm::vec2 from, glm::vec2 to)
  {
    draw_from = from;
    draw_to = to;
  }

private:
  void uniform_setter_callback() override;

  void draw() override;

  void update_global_texture_layers();
  void update_local_texture_layers();

  std::shared_ptr< TerrainMap > terrain_map;

  std::vector< float > texture_layer_indices;

  std::optional< glm::vec2 > start_point;
  std::optional< unsigned > edge_count;

  inline static glm::vec2 draw_from;
  inline static glm::vec2 draw_to;
};
