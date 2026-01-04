#include <cstddef>
#include <mutex>
#include <vector>

#include <glhelp/Error.hpp>
#include <glhelp/Texture.hpp>

#include <hgt/Hgt.hpp>

#include "Setup.hpp"
#include "TerrainMap.hpp"

static auto pbo_to_vector(auto pbo) -> std::vector< ImageData* >
{
  std::vector< ImageData* > image_data_bufer;
  image_data_bufer.reserve(BufferSize);
  for (auto i{0U}; i < BufferSize; i++) {
    image_data_bufer.emplace_back(&pbo.get_layer(i));
  }

  return image_data_bufer;
}

static auto align_to_offset(i16 i, i16 offset, u8 layer) -> i16
{
  if (layer == 0) {
    return i;
  }

  const u16 bit_mask{static_cast< u16 >(~((1 << layer) - 1))};
  return ((i - offset) & bit_mask) + offset;
}

TerrainMap::TerrainMap(const fs::path& path, u8 worker_count, TerrainMapArea map_area)
    : pbo(), hgt_parser(path, worker_count, pbo_to_vector(pbo), true)
{
  glGenTextures(1, &texture_id);
  if (texture_id == 0) {
    throw std::runtime_error("TerrainMap: Failed to generate texture.");
  }

  for (size_t i{0}; i < TerrainLayers; i++) {
    enabled_area[i] = TerrainMapArea{
        .lat_from = align_to_offset(map_area.lat_from, -90, i),
        .lat_to = align_to_offset(map_area.lat_to, -90, i),
        .lon_from = align_to_offset(map_area.lon_from, -180, i),
        .lon_to = align_to_offset(map_area.lon_to, -180, i),
        .layer = static_cast< u8 >(i)};
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R16, HGT_RESOLUTION, HGT_RESOLUTION, TextureLayers);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void TerrainMap::map_area(TerrainMapArea area)
{
  current_time++;

  const auto& allow_area{enabled_area[area.layer]};
  area.lat_from = std::max(area.lat_from, allow_area.lat_from);
  area.lat_to = std::min(area.lat_to, allow_area.lat_to);
  area.lon_from = std::max(area.lon_from, allow_area.lon_from);
  area.lon_to = std::min(area.lon_to, allow_area.lon_to);

  auto map_lock{std::unique_lock< std::mutex >(map_mutex)};
  HgtParserState::for_each_cord(area, [&](i16 lat, i16 lon) {
    const DataCordinates cords{
        .lat = lat,
        .lon = lon,
        .layer = area.layer};

    if (texture_map.contains(cords)) {
      texture_map.at(cords).last_use_time = current_time;
      return;
    }

    texture_map.emplace(cords, TileObject{
                                   .layer = {},
                                   .hgt_data = std::nullopt,
                                   .in_queue = true,
                                   .last_use_time = current_time});

    hgt_parser.add_task(Task{
        .cordinates = cords,
        .callback = [this](HgtData hgt_data) { request_texture_upload(std::move(hgt_data)); }});
  });
}

void TerrainMap::unmap_all()
{
  auto map_lock{std::unique_lock(map_mutex)};
  texture_map.clear();
  for (auto i{0U}; i < TextureLayers; i++) {
    texture_layer_used[i] = false;
  }
}

void TerrainMap::request_texture_upload(HgtData hgt_data)
{
  if (hgt_data.is_zeroed) {
    texture_map.at(hgt_data.cordinates).layer = std::nullopt;
    texture_map.at(hgt_data.cordinates).in_queue = false;
  }
  else {
    auto queue_lock{std::unique_lock< std::mutex >(queue_mutex)};
    upload_queue.push(std::move(hgt_data));
  }
}

void TerrainMap::sync()
{
  std::array< std::optional< HgtData >, 2 > uploads;

  {
    auto queue_lock{std::unique_lock< std::mutex >(queue_mutex)};
    if (upload_queue.empty()) {
      return;
    }

    for (auto i{uploads.size()}; i > 0; i--) {
      uploads[i - 1].emplace(std::move(upload_queue.front()));
      upload_queue.pop();

      if (upload_queue.empty()) {
        break;
      }
    }
  }

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo.id());
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);

  for (auto& upload_opt : uploads) {
    if (!upload_opt.has_value()) {
      continue;
    }

    auto texture_map_lock{std::unique_lock(map_mutex)};
    auto& upload{upload_opt.value()};

    const auto layer{get_texture_layer(texture_map_lock)};
    auto offset{reinterpret_cast< size_t >(&upload.data) - reinterpret_cast< size_t >(&pbo.get_layer(0))};

    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        0, 0,
        layer,
        HGT_RESOLUTION,
        HGT_RESOLUTION,
        1,
        GL_RED,
        GL_UNSIGNED_SHORT,
        reinterpret_cast< void* >(offset));

    texture_map[upload.cordinates].layer = layer;
    texture_map.at(upload.cordinates).hgt_data.emplace(std::move(upload));
    texture_map.at(upload.cordinates).in_queue = false;
  }

  CHECK_GL("TerrainMap: After texture sub");

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

auto TerrainMap::get_texture_layer(std::unique_lock< std::mutex >& texture_map_lock) -> unsigned
{
  for (auto i{0U}; i < TextureLayers; i++) {
    if (!texture_layer_used[i]) {
      texture_layer_used[i] = true;
      return i;
    }
  }

  u64 current_treshold{DefaultTreshold};
  std::optional< unsigned > free_layer;
  while (!(free_layer = unmap_old(current_treshold, texture_map_lock)).has_value()) {
    current_treshold /= 2;
  }

  // This will always be filled.
  // In the worst case we will run with `treshold == 0` which will clear all layers.
  texture_layer_used[free_layer.value()] = true;
  return free_layer.value();
}

auto TerrainMap::unmap_old(u64 treshold, std::unique_lock< std::mutex >& texture_map_lock [[maybe_unused]]) -> std::optional< unsigned >
{
  std::vector< DataCordinates > to_remove;

  for (const auto& [key, value] : texture_map) {
    if (!value.in_queue && value.last_use_time <= current_time - treshold) {
      to_remove.emplace_back(key);
    }
  }

  std::optional< unsigned > reclaimed_layer;
  for (const auto& key : to_remove) {
    if (auto layer{texture_map.at(key).layer}; layer.has_value()) {
      texture_layer_used[layer.value()] = false;
      reclaimed_layer = layer;
    }
    texture_map.erase(key);
  }

  return reclaimed_layer;
}
