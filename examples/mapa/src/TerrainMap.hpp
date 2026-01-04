#pragma once

#include "Setup.hpp"
#include <array>
#include <mutex>
#include <queue>
#include <unordered_map>

#include <glm/glm.hpp>

#include <glhelp/Texture.hpp>

#include <hgt/Hgt.hpp>

using namespace hgt;

constexpr unsigned BufferSize{256};

constexpr unsigned ClearTextureLayer{123456};

constexpr float EMPTY_LAYER{-1.0F};

class TerrainMap : public glhelp::Texture< GL_TEXTURE_2D_ARRAY > {
public:
  TerrainMap(const fs::path& path, u8 worker_count, TerrainMapArea map_area);
  ~TerrainMap() = default;

  TerrainMap(const TerrainMap&) noexcept = delete;
  auto operator=(const TerrainMap&) noexcept -> TerrainMap& = delete;

  void map_area(TerrainMapArea area);
  void unmap_all();

  void request_texture_upload(HgtData hgt_data);
  void sync();

  [[nodiscard]] auto get_layer(DataCordinates cords) -> std::optional< unsigned >
  {
    // WARNING: Lack of mutex here is risky, but its presence introduces severe performance peanalty!
    // auto map_lock{std::unique_lock(map_mutex)};
    if (texture_map.contains(cords) && !texture_map.at(cords).in_queue) {
      return texture_map.at(cords).layer.value_or(ClearTextureLayer);
    }
    return std::nullopt;
  }

  void generate_cache(const TerrainMapArea& map_area) { hgt_parser.generate_cache(map_area); }

  auto get_task_queue() const noexcept -> std::size_t { return hgt_parser.get_queue_length(); }

private:
  [[nodiscard]] auto get_texture_layer(std::unique_lock< std::mutex >& texture_map_lock) -> unsigned;

  static constexpr u64 DefaultTreshold{128};
  auto unmap_old(u64 treshold, std::unique_lock< std::mutex >& texture_map_lock) -> std::optional< unsigned >;

  static constexpr unsigned TextureLayers{BufferSize / 2};

  glhelp::PBOTextureBuffer< u16, HGT_RESOLUTION, HGT_RESOLUTION, BufferSize > pbo;
  HgtParserState hgt_parser;
  std::array< TerrainMapArea, TerrainLayers > enabled_area;

  struct TileObject {
    std::optional< unsigned > layer;
    std::optional< HgtData > hgt_data;
    bool in_queue;
    u64 last_use_time;
  };

  u64 current_time{0};

  std::mutex map_mutex;
  std::unordered_map< DataCordinates, TileObject > texture_map;
  std::array< bool, TextureLayers > texture_layer_used{};

  std::mutex queue_mutex;
  std::queue< HgtData > upload_queue;
};
