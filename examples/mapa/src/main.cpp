#include <exception>
#include <filesystem>
#include <glm/geometric.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

#include <glhelp/Buttons.hpp>
#include <glhelp/Camera.hpp>
#include <glhelp/Error.hpp>
#include <glhelp/Obj.hpp>
#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#include <hgt/Hgt.hpp>

#include "GlobeCamera.hpp"
#include "MapCamera.hpp"
#include "MapTile.hpp"
#include "MapTile3d.hpp"
#include "Setup.hpp"
#include "TerrainMap.hpp"

#ifndef SHADER_DIR_PATH
#error "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

#ifndef OBJ_DIR_PATH
#error "Shader directory undefined. Please define OBJ_DIR_PATH macro."
#endif

struct ProgramSetup {
  fs::path hgt_path;
  bool print_fps{false};
  bool preload_initial{false};
  bool no_cache{false};
  TerrainMapArea map_area{
      .lat_from = 51,
      .lat_to = 52,
      .lon_from = 16,
      .lon_to = 17,
      .layer = MaxLayer};
  unsigned thread_count{6};
};

static void poll_lod_scaling(glhelp::Window& window, float& lod_scaling, float frame_time)
{
  auto callback{[&](bool down, bool up, bool reset) -> bool {
    if (down) {
      lod_scaling -= 0.5F * frame_time;
    }
    if (up) {
      lod_scaling += 0.5F * frame_time;
    }

    if (reset) {
      lod_scaling = 1.0F;
    }

    lod_scaling = std::min(1.0F, std::max(0.05F, lod_scaling));
    return true;
  }};

  glhelp::call_button_function< GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_R >(callback, GLFW_PRESS, window);
}

void run_program(const ProgramSetup& setup)
{
  auto window{std::make_shared< glhelp::Window >(800, 800, "glhelp::map", setup.print_fps, GLFW_CURSOR_NORMAL)};

  // Mercator projection is not linear, 'ugly' fix is to just have vertices on the same positions. That way we get no mismatches.
  auto plane_obj{glhelp::Obj< glhelp::SimpleVertex >::parse_from_file(OBJ_DIR_PATH "plane.obj")};
  auto plane1_obj{glhelp::Obj< glhelp::SimpleVertex >::parse_from_file(OBJ_DIR_PATH "plane1.obj")};
  auto plane2_obj{glhelp::Obj< glhelp::SimpleVertex >::parse_from_file(OBJ_DIR_PATH "plane2.obj")};
  auto plane3_obj{glhelp::Obj< glhelp::SimpleVertex >::parse_from_file(OBJ_DIR_PATH "plane3.obj")};
  auto plane4_obj{glhelp::Obj< glhelp::SimpleVertex >::parse_from_file(OBJ_DIR_PATH "plane4.obj")};

  auto terrain_map{std::make_shared< TerrainMap >(setup.hgt_path, setup.thread_count, setup.map_area)};

  const glm::vec2 initial_position{
      (setup.map_area.lon_from + setup.map_area.lon_to) / 2.0F,
      to_mercator((setup.map_area.lat_from + setup.map_area.lat_to) / 2.0F)};

  const float initial_size{0.7F * std::sqrt(static_cast< float >(std::max(setup.map_area.lon_to - setup.map_area.lon_from, setup.map_area.lat_to - setup.map_area.lat_from)))};
  float lod_scaling{1.0F};

  MapTile::set_draw_boundaries(glm::vec2{setup.map_area.lon_from, setup.map_area.lat_from}, glm::vec2{setup.map_area.lon_to, setup.map_area.lat_to});
  MapTile3d::set_draw_boundaries(glm::vec2{setup.map_area.lon_from, setup.map_area.lat_from}, glm::vec2{setup.map_area.lon_to, setup.map_area.lat_to});

  if (setup.preload_initial) {
    terrain_map->generate_cache(setup.map_area);
  }

  window->fps_callback = [&]() {
    return std::format("cache-generation queue size: {}", terrain_map->get_task_queue());
  };

  /*
   * Setup for top-down map view.
   */
  auto terrain_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "map_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "map_fragment.glsl")})};

  auto map_camera{std::make_shared< MinimapCamera >(
      window,
      initial_position,
      initial_size,
      -200.0F, 200.0F)};

  std::array< std::shared_ptr< MapTile >, TerrainLayers > map_layers{
      std::make_shared< MapTile >(terrain_map, plane_obj, terrain_shader, 0),
      std::make_shared< MapTile >(terrain_map, plane1_obj, terrain_shader, 1),
      std::make_shared< MapTile >(terrain_map, plane2_obj, terrain_shader, 2),
      std::make_shared< MapTile >(terrain_map, plane3_obj, terrain_shader, 3),
      std::make_shared< MapTile >(terrain_map, plane4_obj, terrain_shader, 4)};

  glhelp::Scene map_scene;
  for (auto& ptr : map_layers) {
    map_scene.add_object(ptr);
  }

  auto draw_map_frame{[&, frame_op = static_cast< u8 >(0)](glhelp::Window& window, double time, double frame_time) mutable -> bool {
    map_camera->poll_keyboard_events(window, frame_time);

    auto map_camera_area{map_camera->get_terrain_map(lod_scaling)};

    // Spread the work-load onto several frames to minimize delays.
    constexpr u8 FrameOpLimit{map_layers.size() + 1};
    if (frame_op == 0) {
      map_camera_area.layer = glm::max(glm::min(map_camera_area.layer, static_cast< u8 >(map_layers.size() - 1)), static_cast< u8 >(0));
      terrain_map->map_area(map_camera_area);
    }
    else {
      map_layers.at(frame_op - 1)->update_texture_layers(map_camera_area);
    }
    frame_op++;

    if (frame_op == FrameOpLimit) {
      frame_op = 0;
    }

    map_scene.draw_minimap(*map_camera, window, time, map_camera->get_minimap_scale(), glm::vec2{0.0F}, window.get_size());
    return true;
  }};

  /*
   * Setup for 3D map view.
   */
  auto globe_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "globe_vertex.glsl"),
      glhelp::create_shader_from_file(GL_TESS_CONTROL_SHADER, SHADER_DIR_PATH "globe_tsc.glsl"),
      glhelp::create_shader_from_file(GL_TESS_EVALUATION_SHADER, SHADER_DIR_PATH "globe_tse.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "globe_fragment.glsl"),
  })};

  unsigned glob_inx{0};
  std::array< std::shared_ptr< MapTile3d >, 2 > globe_layers{
      std::make_shared< MapTile3d >(terrain_map, plane_obj, globe_shader, 0, std::pow(2.0F, -4.0F), 90),
      std::make_shared< MapTile3d >(terrain_map, plane_obj, globe_shader, 4, std::pow(2.0F, 4.0F))};

  auto globe_camera{std::make_shared< GlobeCamera >(window,
                                                    GlobePosition(glm::vec2{initial_position.x, initial_position.y}, 1.0F, Radius))};

  glhelp::Scene globe_scene;
  for (auto& ptr : globe_layers) {
    globe_scene.add_object(ptr);
  }

  auto draw_globe_frame{[&](glhelp::Window& window [[maybe_unused]], double time, double frame_time [[maybe_unused]]) mutable -> bool {
    globe_camera->poll_keyboard_events(window, frame_time);
    globe_camera->update_clipping();

    auto& current_layer{globe_layers.at(glob_inx)};
    const auto lat_lon{globe_camera->get_lat_lon()};
    auto map_area{current_layer->move_to_lat_lon(lat_lon)};

    terrain_map->map_area(map_area);
    current_layer->update_texture_layers();
    if (++glob_inx == globe_layers.size()) {
      glob_inx = 0;
    }

    globe_scene.draw_objects(*globe_camera, time);
    return true;
  }};

  /*
   * Main Loop.
   */
  glhelp::TogglingButton map_view(GLFW_KEY_TAB, *window, true);
  auto map_event{map_view.event.connect([&](bool map_view) {
    if (map_view) {
      glfwSetInputMode(window->get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      map_camera->set_lat_lon(globe_camera->get_lat_lon());
      map_camera->enabled = true;
      globe_camera->enabled = false;
    }
    else {
      glfwSetInputMode(window->get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      globe_camera->set_lat_lon(map_camera->get_lat_lon());
      globe_camera->enabled = true;
      map_camera->enabled = false;
    }
  })};

  glhelp::TogglingButton toggle_wireframe(GLFW_KEY_BACKSPACE, *window);
  auto toggle_event{toggle_wireframe.event.connect([&](bool is_pressed) {
    if (is_pressed) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  })};

  glhelp::TogglingButton toggle_heigth_bump(GLFW_KEY_Q, *window);
  auto heigth_bump_event{toggle_heigth_bump.event.connect([&](bool is_pressed) {
    if (is_pressed) {
      MapTile3d::heigth_scale = 5.0F;
    }
    else {
      MapTile3d::heigth_scale = 1.0F;
    }
  })};

  globe_camera->init_mouse(*window);
  window->run_synchronously([&](glhelp::Window& window, double time, double frame_time) -> bool {
    poll_lod_scaling(window, lod_scaling, frame_time);
    MapTile3d::lod_tess = lod_scaling;

    terrain_map->sync();

    if (map_view()) {
      return draw_map_frame(window, time, frame_time);
    }
    else {
      return draw_globe_frame(window, time, frame_time);
    }
  });
}

static void print_help(const char* argv0)
{
  std::cerr << "Usage: " << argv0 << " <hgt-asset-path> [options...]\n"
            << "  <hgt-asset-path>  Path to directory containing HGT files.\n"
            << "  -fps              Enable FPS printing.\n"
            << "  -lat <from> <to>  Sets latitude range (defaults to [51, 52)).\n"
            << "  -lon <from> <to>  Sets longitude range (defaults to 16, 17)).\n"
            << "  -preload          Preload map region and cache it.\n"
            << "  -no-cache         Disables tile cache creation (huge performance impact).\n"
            << "  -proc <n>         How many threads should be used for loading (default is 6).\n"
            << std::flush;
}

auto main(int argc, char* argv[]) -> int
{
  if (argc < 2) {
    std::cerr << "Parameter 'hgt-asset-path' is required!\n";
    print_help(argv[0]);
    return 1;
  }

  if (argv[1] == std::string("--help") || argv[1] == std::string("-h")) {
    print_help(argv[0]);
    return 0;
  }

  ProgramSetup setup{
      .hgt_path = fs::path(argv[1])};

  for (auto i{2}; i < argc; i++) {
    if (argv[i] == std::string("-fps")) {
      setup.print_fps = true;
    }
    else if (argv[i] == std::string("-preload")) {
      setup.preload_initial = true;
    }
    else if (argv[i] == std::string("-lat")) {
      if (i + 2 >= argc) {
        std::cerr << "Missing latitude range arguments!\n";
        print_help(argv[0]);
        return 1;
      }
      setup.map_area.lat_from = std::stof(argv[++i]);
      setup.map_area.lat_to = std::stof(argv[++i]);
      if (setup.map_area.lat_from > setup.map_area.lat_to) {
        std::cerr << "Invalid latitude range!\n";
        print_help(argv[0]);
        return 1;
      }
    }
    else if (argv[i] == std::string("-lon")) {
      if (i + 2 >= argc) {
        std::cerr << "Missing longitude range arguments!\n";
        print_help(argv[0]);
        return 1;
      }
      setup.map_area.lon_from = std::stof(argv[++i]);
      setup.map_area.lon_to = std::stof(argv[++i]);
      if (setup.map_area.lon_from > setup.map_area.lon_to) {
        std::cerr << "Invalid longitude range!\n";
        print_help(argv[0]);
        return 1;
      }
    }
    else if (argv[i] == std::string("-proc")) {
      if (i + 1 >= argc) {
        std::cerr << "Missing proc argument!\n";
        print_help(argv[0]);
        return 1;
      }
      setup.thread_count = static_cast< unsigned >(std::stoi(argv[++i]));
      if (setup.thread_count < 1 || setup.thread_count > 16) {
        std::cerr << "Invalid proc count! Allowed range is [1-16]\n";
        print_help(argv[0]);
        return 1;
      }
    }
    else {
      std::cerr << "Unknown parameter: " << argv[i] << '\n';
      print_help(argv[0]);
      return 1;
    }
  }

  try {
    glhelp::GLFWContext context;
    run_program(setup);
  }
  catch (std::exception& e) {
    std::cerr << "Program crashed!:\n"
              << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
