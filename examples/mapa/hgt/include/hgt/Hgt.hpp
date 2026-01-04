#pragma once

#include <array>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#define HGT_RESOLUTION static_cast< u16 >(1201)
#define HGT_OFFSET static_cast< u16 >(500)

#ifdef DEBUG_GLHELP
#define INFO_LOG(msg) std::cerr << "[INFO] Hgt: " << msg << '\n';
#define WARNING_LOG(msg) std::cerr << "[WARNING] Hgt: " << msg << '\n';
#else
#define INFO_LOG(msg)
#define WARNING_LOG(msg)
#endif

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

namespace hgt {

namespace fs = std::filesystem;
using ImageData = std::array< std::array< u16, HGT_RESOLUTION >, HGT_RESOLUTION >;

struct DataCordinates {
  i16 lat;
  i16 lon;
  u8 layer;

  auto operator==(const DataCordinates& other) const -> bool
  {
    return lat == other.lat && lon == other.lon && layer == other.layer;
  }
};

struct DataPack {
  DataCordinates cordinates;
  u16 user_count;
  // Data cleanup will be handled by some worker if we start running out of space.
  ImageData* data;
  // Set when the data pack contains no valid data (e.g. file not found).
  // In that case the `data` is set to zeroed ImageData.
  bool empty_data;
};

} // namespace hgt

template<>
struct std::hash< hgt::DataCordinates > {
  using T = hgt::DataCordinates;
  auto operator()(const T& pack) const -> std::size_t
  {
    return pack.layer * ((static_cast< u64 >(pack.lat)) | (static_cast< u64 >(pack.lon) << (8 * sizeof(T::lon))));
  }
};

namespace hgt {

struct TerrainMapArea {
  i16 lat_from;
  i16 lat_to;
  i16 lon_from;
  i16 lon_to;
  u8 layer;

  auto operator==(TerrainMapArea& other) -> bool
  {
    return lat_from == other.lat_from && lat_to == other.lat_to && lon_from == other.lon_from && lon_to == other.lon_to && layer == other.layer;
  }
};

class HgtParserState {
public:
  // The `preallocated_image_data` is not managed by HgtParser.
  HgtParserState(const fs::path& path, u8 worker_count, std::vector< ImageData* >&& preallocated_image_data, bool generate_cache = true);

  HgtParserState(const HgtParserState&) = delete;
  auto operator=(const HgtParserState&) -> HgtParserState& = delete;

  ~HgtParserState();

  struct HgtData {
    HgtData(DataCordinates cordinates, ImageData& data, bool is_zeroed, HgtParserState& hgt_parser);

    HgtData(HgtData&& other) noexcept;
    auto operator=(HgtData&& other) noexcept -> HgtData&;

    HgtData(const HgtData&) = delete;
    auto operator=(const HgtData) -> HgtData = delete;

    ~HgtData();

    DataCordinates cordinates;
    ImageData& data;
    bool is_zeroed;

  private:
    void decrease_user_count();

    std::reference_wrapper< HgtParserState > hgt_parser;
    bool owns_data{true};
  };

  struct Task {
    DataCordinates cordinates;
    std::function< void(HgtData) > callback;
    /// Scaling factor for a data sample.
    /// Value of `n` will combine `n*n` samples into a single image.
    std::optional< std::tuple< std::shared_ptr< std::atomic< u16 > >, ImageData* > > scaled_read_data{};

    [[nodiscard]] auto to_string() const -> std::string;
  };

  /// Add a task for workers.
  void add_task(Task&& task);

  void generate_cache(const TerrainMapArea& map_area);
  bool gen_cache;

  static void for_each_cord(TerrainMapArea area, const std::function< void(i16, i16) >& f);

  auto get_queue_length() const noexcept -> std::size_t { return queue_length; }

private:
  auto file_from_cords(const DataCordinates& cords) const -> fs::path;
  [[nodiscard]] auto should_load_from_file(const DataCordinates& cords) const -> bool;

  [[nodiscard]] auto handle_data_present(u8 thread_id, Task& task, std::unique_lock< std::mutex >& data_pack_lock) -> std::optional< ImageData* >;
  [[nodiscard]] auto attempt_image_load(u8 thread_id, Task& task, std::unique_lock< std::mutex >& data_pack_lock) -> std::optional< ImageData* >;

  auto read_file(const DataCordinates& cords) -> ImageData*;

  auto acquire_image_data_space() -> ImageData*;
  auto clean_data_pack(std::unique_lock< std::mutex >& image_data_lock) -> bool;

  /// `Task` entry-point.
  void task_main(u8 thread_id);
  /// Block untill a task is available.
  /// On stop-request returns `std::nullopt`. Nothing will be returned later.
  auto get_next_task() -> std::optional< Task >;
  [[nodiscard]] auto schedule_image_loaders(Task& task) -> bool;

  fs::path base_path;
  ImageData* zeroed_image;

  std::vector< std::thread > workers;
  std::condition_variable condition;

  // Queue of image loading tasks.
  std::mutex queue_mutex;
  std::queue< Task > queue;

  // List of available image spaces (1MB segments).
  std::mutex image_data_mutex;
  std::vector< ImageData* > image_data_space;

  std::mutex data_pack_mutex;
  // Container for all data packs.
  // Upon creation request an empty optional is inserted to signal "loading in progress".
  std::unordered_map< DataCordinates, std::optional< DataPack > > data;

  bool thread_stop{false};

  std::size_t queue_length;
};

using HgtData = HgtParserState::HgtData;
using Task = HgtParserState::Task;

} // namespace hgt
