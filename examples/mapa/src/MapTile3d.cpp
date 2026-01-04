#include <vector>

#include <glm/glm.hpp>

#include <glhelp/mesh/InstancedMesh3D.hpp>

#include <hgt/Hgt.hpp>

#include "MapTile3d.hpp"
#include "Setup.hpp"

constexpr auto tile_count(unsigned layer) -> size_t
{
  return 2 * 90 * 2 * 180 / (1 << layer);
}

static auto get_global_positions(unsigned layer) -> std::vector< glm::vec3 >
{
  std::vector< glm::vec3 > tile_positions;
  tile_positions.reserve(tile_count(layer));

  const auto scaling{1 << layer};

  for (i16 lat{0}; lat < (180 + scaling - 1) / scaling; lat++) {
    for (i16 lon{0}; lon < (360 + scaling - 1) / scaling; lon++) {
      tile_positions.emplace_back(
          static_cast< float >(lon),
          static_cast< float >(lat),
          0.0F);
    }
  }

  return tile_positions;
}

MapTile3d::MapTile3d(
    std::shared_ptr< TerrainMap > terrain_map,
    const obj_parser::Obj< obj_parser::SimpleVertex >& plane,
    std::shared_ptr< glhelp::ShaderProgram > shader_program,
    u8 texture_layer,
    float tile_scale)
    : glhelp::InstancedMesh3d< glhelp::CachingSimplePosition, glm::vec3, float >(
          glhelp::CachingSimplePosition{},
          std::move(shader_program),
          plane, {get_global_positions(texture_layer), std::vector< float >(tile_count(texture_layer), EMPTY_LAYER)}),
      tile_scale(tile_scale),
      texture_layer(texture_layer),
      terrain_map(std::move(terrain_map)),
      texture_layer_indices(tile_count(texture_layer), EMPTY_LAYER),
      start_point(std::nullopt),
      edge_count(std::nullopt)
{
}

static auto get_positions(i16 edge_count) -> std::vector< glm::vec3 >
{
  std::vector< glm::vec3 > tile_positions;
  tile_positions.reserve(edge_count * edge_count);

  for (i16 lat{0}; lat < edge_count; lat++) {
    for (i16 lon{0}; lon < edge_count; lon++) {
      tile_positions.emplace_back(
          static_cast< float >(lon),
          static_cast< float >(lat),
          0.0F);
    }
  }

  return tile_positions;
}

MapTile3d::MapTile3d(
    std::shared_ptr< TerrainMap > terrain_map,
    const obj_parser::Obj< obj_parser::SimpleVertex >& plane,
    std::shared_ptr< glhelp::ShaderProgram > shader_program,
    u8 texture_layer,
    float tile_scale,
    const unsigned tiles_edge)
    : glhelp::InstancedMesh3d< glhelp::CachingSimplePosition, glm::vec3, float >(
          glhelp::CachingSimplePosition{},
          std::move(shader_program),
          plane, {get_positions(tiles_edge), std::vector< float >(tiles_edge * tiles_edge, EMPTY_LAYER)}),
      tile_scale(tile_scale),
      texture_layer(texture_layer),
      terrain_map(std::move(terrain_map)),
      texture_layer_indices(tiles_edge * tiles_edge, EMPTY_LAYER),
      start_point({0.0F, 0.0F}),
      edge_count(tiles_edge)
{
}

static auto align_to_offset(i16 i, i16 offset, u8 layer) -> i16
{
  if (layer == 0) {
    return i;
  }

  const u16 bit_mask{static_cast< u16 >(~((1 << layer) - 1))};
  return ((i - offset) & bit_mask) + offset;
}

static auto align_to_offsetf(float f, float step) -> float
{
  return std::floor(f / step) * step;
}

auto MapTile3d::move_to_lat_lon(glm::vec2 lat_lon) -> TerrainMapArea
{
  camera_lat_lon = lat_lon;

  if (!start_point.has_value()) {
    return TerrainMapArea{
        .lat_from = -90,
        .lat_to = 90,
        .lon_from = -180,
        .lon_to = 180,
        .layer = texture_layer};
  }

  if (tile_scale >= 1.0F) {
    start_point = glm::vec2{
        align_to_offset(lat_lon.x - (tile_scale / 2.0F) * (float)edge_count.value(), -180, texture_layer),
        align_to_offset(lat_lon.y - (tile_scale / 2.0F) * (float)edge_count.value(), -90, texture_layer)};
  }
  else {
    start_point = glm::vec2{
        align_to_offsetf(lat_lon.x - (tile_scale / 2.0F) * (float)edge_count.value(), tile_scale),
        align_to_offsetf(lat_lon.y - (tile_scale / 2.0F) * (float)edge_count.value(), tile_scale),
    };
  }

  TerrainMapArea area{
      .lat_from = static_cast< i16 >(start_point->y),
      .lat_to = std::min(static_cast< i16 >(start_point->y + edge_count.value() * tile_scale), static_cast< i16 >(89)),
      .lon_from = static_cast< i16 >(start_point->x),
      .lon_to = std::min(static_cast< i16 >(start_point->x + edge_count.value() * tile_scale), static_cast< i16 >(89)),
      .layer = texture_layer,
  };

  return area;
}

void MapTile3d::update_texture_layers()
{
  if (!start_point.has_value()) {
    update_global_texture_layers();
  }
  else {
    update_local_texture_layers();
  }
}

void MapTile3d::uniform_setter_callback()
{
  terrain_map->load_to_texture_unit(0);
  shader->set_uniform< glhelp::Texture< GL_TEXTURE_2D_ARRAY > >("map", *terrain_map);
  shader->set_uniform< float >("MaxTess", std::min(MaxTessLevel * lod_tess, MaxTessLevel));
  shader->set_uniform< float >("tile_scale", tile_scale);
  shader->set_uniform< glm::vec2 >("start_point", start_point.value_or(glm::vec2{-180.0F, -90.0F}));
  shader->set_uniform< glm::vec2 >("camera_lat_lon", camera_lat_lon);
  shader->set_uniform< float >("heigthScale", heigth_scale);
  shader->set_uniform("draw_from", draw_from);
  shader->set_uniform("draw_to", draw_to);
}

void MapTile3d::draw()
{
  glBindVertexArray(this->vao);

  const auto model_matrix{get_model_matrix(*this)};
  this->shader->set_uniform("uModelTransform", model_matrix);

  this->uniform_setter_callback();

  glDrawElementsInstanced(GL_PATCHES, this->indices_count, GL_UNSIGNED_INT, nullptr, instance_count);

  glBindVertexArray(0);
}

void MapTile3d::update_global_texture_layers()
{
  auto map_area{TerrainMapArea{
      .lat_from = -90,
      .lat_to = 89,
      .lon_from = -180,
      .lon_to = 179,
      .layer = texture_layer}};
  const auto scaling{1 << texture_layer};

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
        .layer = texture_layer};

    const auto layer_opt{terrain_map->get_layer(cords)};
    texture_layer_indices.at(get_inx(lat_inx, lon_inx)) = layer_opt.has_value()
                                                              ? *layer_opt
                                                              : EMPTY_LAYER;
  });

  update_buffer_instance_data< float >(texture_layer_indices);
}

void MapTile3d::update_local_texture_layers()
{
  for (unsigned x{0}; x < edge_count.value(); x++) {
    for (unsigned y{0}; y < edge_count.value(); y++) {
      glm::vec2 pos{
          start_point->x + x * tile_scale,
          start_point->y + y * tile_scale};

      const auto lat{align_to_offset(pos.y, -90, texture_layer)};
      const auto lon{align_to_offset(pos.x, -180, texture_layer)};
      const DataCordinates cords{
          .lat = lat,
          .lon = lon,
          .layer = texture_layer};

      const auto layer_opt{terrain_map->get_layer(cords)};
      const auto index = y * edge_count.value() + x;
      texture_layer_indices.at(index) = layer_opt.has_value()
                                            ? *layer_opt
                                            : EMPTY_LAYER;
    }
  }

  update_buffer_instance_data< float >(texture_layer_indices);
}
