#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <hgt/Hgt.hpp>

namespace hgt {

HgtParserState::HgtData::HgtData(DataCordinates cordinates, ImageData& data, bool is_zeroed, class HgtParserState& hgt_parser)
    : cordinates(cordinates), data(data), is_zeroed(is_zeroed), hgt_parser(hgt_parser)
{
}

HgtParserState::HgtData::HgtData(HgtParserState::HgtData&& other) noexcept
    : cordinates(other.cordinates),
      data(other.data),
      is_zeroed(other.is_zeroed),
      hgt_parser(other.hgt_parser),
      owns_data(other.owns_data)
{
  other.owns_data = false;
}

auto HgtParserState::HgtData::operator=(HgtParserState::HgtData&& other) noexcept -> HgtParserState::HgtData&
{
  if (this != &other) {
    decrease_user_count();

    cordinates = other.cordinates;
    data = other.data;
    is_zeroed = other.is_zeroed;
    hgt_parser = other.hgt_parser;
    owns_data = other.owns_data;

    other.owns_data = false;
  }

  return *this;
}

HgtParserState::HgtData::~HgtData()
{
  decrease_user_count();
}

void HgtParserState::HgtData::decrease_user_count()
{
  if (owns_data) {
    auto& this_parser{hgt_parser.get()};
    this_parser.data.at(cordinates).value().user_count--;
  }
}

auto HgtParserState::Task::to_string() const -> std::string
{
  return std::format("Task(lat: {}, lon: {}, layer: {})", cordinates.lat, cordinates.lon, cordinates.layer);
}

HgtParserState::HgtParserState(const fs::path& path, u8 worker_count, std::vector< ImageData* >&& preallocated_image_data, bool generate_cache)
    : gen_cache(generate_cache), base_path(path), image_data_space(std::move(preallocated_image_data))
{
  if (!fs::exists(path)) {
    throw std::runtime_error("HgtParserState: Provided path does not exist.");
  }

  if (!fs::is_directory(path)) {
    throw std::runtime_error("HgtParserState: Provided path does not point to a directory.");
  }

  if (2 * worker_count > image_data_space.size()) {
    throw std::runtime_error("HgtParserState: worker_count should be at least twice as high as max_data_packs to support stable computation.");
  }

  if (generate_cache && !fs::exists(path / ".cache")) {
    fs::create_directory(path / ".cache");
  }

  zeroed_image = image_data_space.back();
  image_data_space.pop_back();
  for (auto& row : *zeroed_image) {
    for (auto& cell : row) {
      cell = 500;
    }
  }

  workers.reserve(worker_count);
  for (u8 i = 0; i < worker_count; i++) {
    workers.emplace_back([this, i]() { this->task_main(i); });
  }
}

HgtParserState::~HgtParserState()
{
  thread_stop = true;
  condition.notify_all();

  for (auto& worker : workers) {
    worker.join();
  }
}

void HgtParserState::add_task(Task&& task)
{
  {
    std::lock_guard lock(queue_mutex);
    queue.push(std::move(task));
  }
  condition.notify_one();
}

#define ANSI_ESCAPE "\r"
#define ANSI_CLEAR_LINE "\033[2K"

void HgtParserState::generate_cache(const TerrainMapArea& map_area)
{
  auto thread_counter{std::make_shared< std::atomic< u32 > >()};
  auto cache_save_callback{[this, thread_counter](HgtData hgt_data) {
    auto path{file_from_cords(hgt_data.cordinates)};
    if (fs::exists(path)) {
      (*thread_counter)++;
      return;
    }

    std::ofstream cache_file(path, std::ios::binary);
    if (!cache_file.is_open()) {
      WARNING_LOG(std::format("Failed to open cache file for {}", path.string()));
      (*thread_counter)++;
      return;
    }

    cache_file.write(reinterpret_cast< const char* >(hgt_data.data.data()), sizeof(hgt_data.data));
    cache_file.close();
    (*thread_counter)++;
  }};

  u16 thread_target{0};
  for (u8 l{1}; l <= map_area.layer; l++) {
    auto map_area_layer{map_area};
    map_area_layer.layer = l;
    for_each_cord(map_area_layer, [&](i16 lat, i16 lon) {
      thread_target++;
      add_task(Task{
          .cordinates = DataCordinates{
              .lat = lat,
              .lon = lon,
              .layer = l},
          .callback = cache_save_callback});
    });

    while (thread_target != *thread_counter) {
      std::cout << ANSI_ESCAPE ANSI_CLEAR_LINE
                << std::format("Generating cache for layer {}, current progress: {} / {}", l, static_cast< u32 >((*thread_counter)), thread_target)
                << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::cout << ANSI_ESCAPE ANSI_CLEAR_LINE
              << std::format("Finished generating cache for layer {}\n", l)
              << std::flush;

    thread_target = 0;
    *thread_counter = 0;
  }

  std::cout << "Cache Generation done!\n"
            << std::flush;
}

static auto align_to_offset(i16 i, i16 offset, u8 layer) -> i16
{
  if (layer == 0) {
    return i;
  }

  const u16 bit_mask{static_cast< u16 >(~((1 << layer) - 1))};
  return ((i - offset) & bit_mask) + offset;
}

void HgtParserState::for_each_cord(TerrainMapArea map_area, const std::function< void(i16, i16) >& f)
{
  const u8 layer{map_area.layer};
  map_area.lat_from = std::max(map_area.lat_from - 1, -90);
  map_area.lat_to = std::min(map_area.lat_to + 1, 90);
  map_area.lon_from = std::max(map_area.lon_from - 1, -180);
  map_area.lon_to = std::min(map_area.lon_to + 1, 180);

  const i16 lat_from{align_to_offset(map_area.lat_from, -90, layer)};
  const i16 lon_from{align_to_offset(map_area.lon_from, -180, layer)};
  const auto scaling{(1 << layer)};

  for (i16 lat{lat_from}; lat < map_area.lat_to; lat += scaling) {
    for (i16 lon{lon_from}; lon < map_area.lon_to; lon += scaling) {
      f(lat, lon);
    }
  }
}

void HgtParserState::task_main(u8 thread_id [[maybe_unused]])
{
  while (true) {
    Task task;
    if (auto next_task{get_next_task()}; next_task.has_value()) {
      task = std::move(next_task.value());
      INFO_LOG(std::format("Thread({}): Received {}). Queue size: {}", (u32)thread_id, task.to_string(), queue.size()));
    }
    else {
      // No task? -> thread stop requested.
      INFO_LOG(std::format("Thread({}): exiting.", (u32)thread_id));
      return;
    }

    std::optional< ImageData* > result_data;
    {
      std::unique_lock data_pack_lock(data_pack_mutex);
      if (data.contains(task.cordinates)) {
        result_data = handle_data_present(thread_id, task, data_pack_lock);
      }
      else {
        data.emplace(task.cordinates, std::nullopt);
        data_pack_lock.unlock();

        if (should_load_from_file(task.cordinates)) {
          result_data = attempt_image_load(thread_id, task, data_pack_lock);
        }
        else {
          // Load multiple images: schedule them as 'tasks'.
          INFO_LOG(std::format("Thread({}): {}: scheduling image-loaders.", thread_id, task.to_string()));
          if (!schedule_image_loaders(task)) {
            data_pack_lock.lock();
            data.erase(task.cordinates);
            add_task(std::move(task));
          }
        }
      }
    }

    // Run callback without 'data_pack' mutex.
    if (result_data.has_value()) {
      INFO_LOG(std::format("Thread({}): callback-call for {}", thread_id, task.to_string()));
      task.callback(HgtData(task.cordinates, *result_data.value(), result_data.value() == zeroed_image, *this));
    }
  }
}

/* Data present or data-load started by other worker.
 * - If present increase the user-count and call the callback.
 * - If not present (but dispached!) add current task to the back of the queue.
 *   This way we avoid multiple workers loading the same data-pack.
 */
auto HgtParserState::handle_data_present(u8 thread_id [[maybe_unused]], Task& task, std::unique_lock< std::mutex >& data_pack_lock) -> std::optional< ImageData* >
{
  if (auto& data_pack_opt{data.at(task.cordinates)}; data_pack_opt.has_value()) {
    INFO_LOG(std::format("{} found in cache!", task.to_string()));

    auto& data_pack{data_pack_opt.value()};
    data_pack.user_count++;
    return data_pack.data;
  }
  else {
    INFO_LOG(std::format("Thread({}): {} is being loaded by other thread, re-scheduling task.", thread_id, task.to_string()));
    data_pack_lock.unlock();
    add_task(std::move(task));
    return std::nullopt;
  }
}

auto HgtParserState::attempt_image_load(u8 thread_id [[maybe_unused]], Task& task, std::unique_lock< std::mutex >& data_pack_lock) -> std::optional< ImageData* >
{
  auto image_data{read_file(task.cordinates)};
  data_pack_lock.lock();

  if (image_data != nullptr) {
    const bool is_zeroed{(image_data == zeroed_image)};
    auto data_pack{DataPack{
        .cordinates = task.cordinates,
        .user_count = 1,
        .data = image_data,
        .empty_data = is_zeroed,
    }};

    data[task.cordinates].emplace(data_pack);
    return data_pack.data;
  }
  else {
    WARNING_LOG(std::format("Thread({}): {}: failed to load. Adding to a queue for a retry.", thread_id, task.to_string()));
    data.erase(task.cordinates);
    data_pack_lock.unlock();
    add_task(std::move(task));
    return std::nullopt;
  }
}

[[nodiscard]] auto HgtParserState::should_load_from_file(const DataCordinates& cords) const -> bool
{
  return cords.layer == 0 || fs::exists(file_from_cords(cords));
}

auto HgtParserState::get_next_task() -> std::optional< Task >
{
  auto lock{std::unique_lock< std::mutex >(queue_mutex)};
  condition.wait(lock, [this]() {
    return thread_stop || !queue.empty();
  });

  if (queue.empty())
    return std::nullopt;

  auto task{queue.front()};
  queue.pop();
  queue_length = queue.size();
  return task;
}

auto HgtParserState::schedule_image_loaders(Task& task) -> bool
{
  auto read_counter{std::make_shared< std::atomic< u16 > >(0)};
  auto image_data_ptr{acquire_image_data_space()};
  if (image_data_ptr == nullptr) {
    return false;
  }

  std::memset(image_data_ptr->data(), 0, sizeof(*image_data_ptr));

  task.scaled_read_data.emplace(read_counter, image_data_ptr);
  const auto& cords{task.cordinates};
  auto shared_task{std::make_shared< Task >(std::move(task))};

  auto scaled_image_cords{[this, read_counter, image_data_ptr, cords, shared_task](HgtData hgt_data) {
    auto& image_data{*image_data_ptr};
    const auto lat_offset{cords.lat != hgt_data.cordinates.lat ? static_cast< u16 >(0) : HGT_RESOLUTION};
    const auto lon_offset{cords.lon == hgt_data.cordinates.lon ? static_cast< u16 >(0) : HGT_RESOLUTION};

    for (i16 lat{0}; lat < HGT_RESOLUTION; lat++) {
      for (i16 lon{0}; lon < HGT_RESOLUTION; lon++) {
        image_data[(lat + lat_offset) / 2][(lon + lon_offset) / 2] += hgt_data.data[lat][lon];
      }
    }

    *read_counter += 1;
    INFO_LOG(std::format("scale-callback: Finished reading image {} out of 4", (u16)(*read_counter)));

    if (*read_counter == 4) {
      // This is the last image needed for processing!
      auto data_pack_lock{std::unique_lock< std::mutex >(data_pack_mutex)};

      if (data[cords].has_value())
        return;

      INFO_LOG(std::format("scale-callback filling in DataPack for Task(lat: {}, lon: {}, layer: {})", cords.lat, cords.lon, cords.layer));
      data[cords].emplace(DataPack{
          .cordinates = cords,
          .user_count = 1,
          .data = image_data_ptr,
          .empty_data = true,
      });
      data_pack_lock.unlock();

      // Normalize the image data
      for (i16 lat{0}; lat < HGT_RESOLUTION; lat++) {
        for (i16 lon{0}; lon < HGT_RESOLUTION; lon++) {
          image_data.at(lat).at(lon) /= 4;
        }
      }

      data[cords].value().empty_data = false;

      const auto file_path{file_from_cords(cords)};
      if (gen_cache && !fs::exists(file_path)) {
        std::ofstream cache_file(file_path, std::ios::binary);
        if (!cache_file.is_open()) {
          WARNING_LOG(std::format("Failed to open cache file for {}", file_path.string()));
        }

        cache_file.write(reinterpret_cast< const char* >(image_data.data()), sizeof(image_data));
        cache_file.close();
      }

      shared_task->callback(HgtData(shared_task->cordinates, image_data, false, *this));
    }
  }};

  const auto offset{1 << (cords.layer - 1)};
  const std::array< DataCordinates, 4 > left_upper_cords{
      DataCordinates{
          .lat = cords.lat,
          .lon = cords.lon,
          .layer = static_cast< u8 >(cords.layer - 1)},
      DataCordinates{
          .lat = static_cast< i16 >(cords.lat + offset),
          .lon = cords.lon,
          .layer = static_cast< u8 >(cords.layer - 1)},
      DataCordinates{
          .lat = cords.lat,
          .lon = static_cast< i16 >(cords.lon + offset),
          .layer = static_cast< u8 >(cords.layer - 1)},
      DataCordinates{
          .lat = static_cast< i16 >(cords.lat + offset),
          .lon = static_cast< i16 >(cords.lon + offset),
          .layer = static_cast< u8 >(cords.layer - 1)}};

  // Do not use 'add_task' to minimize mutex operations
  {
    auto lock{std::unique_lock(queue_mutex)};
    for (const auto& cords : left_upper_cords) {
      queue.emplace(Task{
          .cordinates = cords,
          .callback = scaled_image_cords,
      });
    }
  }

  condition.notify_one();
  return true;
}

auto HgtParserState::read_file(const DataCordinates& cords) -> ImageData*
{
  fs::path file_path{file_from_cords(cords)};
  if (!fs::exists(file_path)) {
    WARNING_LOG(std::format("read_file({}): File not found! Returning zeroed_image.", file_path.filename().string()));
    return zeroed_image;
  }

  auto image_data{acquire_image_data_space()};
  if (image_data == nullptr) {
    return nullptr;
  }

  std::ifstream file(file_path, std::ios::binary);
  if (!file.is_open()) {
    WARNING_LOG(std::format("read_file({}): Failed to open file! Returning zeroed_image.", file_path.filename().string()));
    return zeroed_image;
  }

  file.read(reinterpret_cast< char* >(image_data->data()), sizeof(*image_data));
  file.close();

  // Only the base layer is encoded in big-endian.
  if (cords.layer == 0) {
    for (auto& arr : *image_data) {
      for (auto& value : arr) {
        u16 uvalue{static_cast< u16 >(value)};
        // HGT files are big-endian.
        uvalue = ((uvalue << 8) | (uvalue >> 8)) + HGT_OFFSET;
        // Error-detection (original data is in range [-500, 9000]).
        if (uvalue > 9500) {
          value = HGT_OFFSET + 1;
        }
        else {
          value = uvalue;
        }
      }
    }
  }

  file.close();

  INFO_LOG(std::format("read_file({}): Success.", file_path.filename().string()));

  return image_data;
}

auto HgtParserState::file_from_cords(const DataCordinates& cords) const -> fs::path
{
  std::string result;
  result += cords.layer > 0 ? std::to_string(cords.layer) : "";
  result += (cords.lat >= 0) ? 'N' : 'S';
  result += std::format("{:02}", std::abs(cords.lat));
  result += (cords.lon >= 0) ? 'E' : 'W';
  result += std::format("{:03}", std::abs(cords.lon));
  result += ".hgt";

  if (cords.layer > 0) {
    return base_path / std::string(".cache") / result;
  }
  else {
    return base_path / result;
  }
}

auto HgtParserState::acquire_image_data_space() -> ImageData*
{
  auto pop_image_data_space{[this]() {
    auto data_ptr{image_data_space.back()};
    image_data_space.pop_back();
    return data_ptr;
  }};

  auto image_data_lock{std::unique_lock< std::mutex >(image_data_mutex)};
  if (!image_data_space.empty()) {
    INFO_LOG("acquire_image_data_space: Re-using old allocated space");
    return pop_image_data_space();
  }

  clean_data_pack(image_data_lock);
  if (!image_data_space.empty()) {
    return pop_image_data_space();
  }

  WARNING_LOG("Failed to find space!");
  return nullptr;
}

auto HgtParserState::clean_data_pack(std::unique_lock< std::mutex >& image_data_lock [[maybe_unused]]) -> bool
{
  auto data_pack_lock{std::unique_lock< std::mutex >(data_pack_mutex)};
  std::vector< DataCordinates > empty_packs;
  for (auto& [cordinates, data_pack_opt] : data) {
    if (data_pack_opt.has_value() && !data_pack_opt.value().empty_data) {
      if (data_pack_opt.value().user_count == 0) {
        empty_packs.push_back(cordinates);
        image_data_space.emplace_back(data_pack_opt.value().data);
        data_pack_opt = std::nullopt;
      }
    }
  }

  for (auto& cordinates : empty_packs) {
    data.erase(cordinates);
  }

  INFO_LOG(std::format("cleaning data_packs, restored {} image data locations", empty_packs.size()));
  return !empty_packs.empty();
}

} // namespace hgt
