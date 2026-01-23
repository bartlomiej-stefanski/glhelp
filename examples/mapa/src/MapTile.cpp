#include <glm/glm.hpp>

#include <glhelp/mesh/InstancedMesh3D.hpp>

#include "MapTile.hpp"

constexpr auto tile_count(unsigned layer) -> size_t
{
  return 2 * 90 * 2 * 180 / (1 << layer);
}

static auto get_positions(unsigned layer) -> std::vector< glm::vec3 >
{
  std::vector< glm::vec3 > tile_positions;
  tile_positions.reserve(tile_count(layer));

  for (i16 lat{-90}; lat < 90; lat += (1 << layer)) {
    for (i16 lon{-180}; lon < 180; lon += (1 << layer)) {
      tile_positions.emplace_back(
          static_cast< float >(lon),
          static_cast< float >(lat),
          static_cast< float >(64 - layer));
    }
  }

  return tile_positions;
}

MapTile::MapTile(
    std::shared_ptr< TerrainMap > terrain_map,
    const glhelp::Obj< glhelp::SimpleVertex >& plane,
    std::shared_ptr< glhelp::ShaderProgram > shader_program,
    u8 layer)
    : glhelp::InstancedMesh3d< glhelp::CachingSimplePosition, glm::vec3, float >(
          glhelp::CachingSimplePosition{},
          std::move(shader_program),
          plane, {get_positions(layer), std::vector< float >(tile_count(layer), EMPTY_LAYER)}),
      layer(layer),
      terrain_map(std::move(terrain_map)),
      texture_layer_indices(tile_count(layer), EMPTY_LAYER)
{
}

void MapTile::update_texture_layers(TerrainMapArea map_area [[maybe_unused]])
{
  map_area.layer = layer;

  const auto scaling{(1 << layer)};
  auto get_inx{[scaling](i16 lat_inx, i16 lon_inx) {
    const auto lon_elements{(2 * 180 + scaling - 1) / scaling};
    return lat_inx * lon_elements + lon_inx;
  }};

  HgtParserState::for_each_cord(map_area, [&](i16 lat, i16 lon) {
    const auto lat_inx{static_cast< i16 >((lat + 90) / scaling)};
    const auto lon_inx{static_cast< i16 >((lon + 180) / scaling)};

    const DataCordinates cords{
        .lat = lat,
        .lon = lon,
        .layer = layer};

    const auto layer_opt{terrain_map->get_layer(cords)};
    texture_layer_indices.at(get_inx(lat_inx, lon_inx)) = layer_opt.has_value()
                                                              ? *layer_opt
                                                              : EMPTY_LAYER;
  });

  update_buffer_instance_data< float >(texture_layer_indices);
}

void MapTile::uniform_setter_callback()
{
  terrain_map->load_to_texture_unit(0);
  shader->set_uniform< glhelp::Texture< GL_TEXTURE_2D_ARRAY > >("map", *terrain_map);
  shader->set_uniform< float >("map_layer", (1 << layer));
  shader->set_uniform("draw_from", draw_from);
  shader->set_uniform("draw_to", draw_to);
}
