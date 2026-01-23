#include <exception>
#include <iostream>
#include <memory>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glhelp/Buttons.hpp>
#include <glhelp/Camera.hpp>
#include <glhelp/Obj.hpp>
#include <glhelp/Scene.hpp>
#include <glhelp/Shader.hpp>
#include <glhelp/Vertex.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/ligting/DirectionalLight.hpp>
#include <glhelp/ligting/SpotLight.hpp>
#include <glhelp/mesh/Mesh3D.hpp>
#include <glhelp/position/Position.hpp>
#include <glhelp/utils/Event.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#include "Bubbles.hpp"
#include "Cube.hpp"
#include "Drone.hpp"
#include "Mountain.hpp"

#ifndef SHADER_DIR_PATH
#error "Shader directory undefined. Please define SHADER_DIR_PATH macro."
#endif

#ifndef OBJ_DIR_PATH
#error "Shader directory undefined. Please define OBJ_DIR_PATH macro."
#endif

void run_program(std::mt19937& rng, int n, bool reduce_geometry)
{
  std::shared_ptr< glhelp::Window > window{std::make_shared< glhelp::Window >(800, 800, "OpenGL simple 3d example")};

  auto bubble_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "bubble_vertex.glsl"),
      glhelp::create_shader_from_file(GL_GEOMETRY_SHADER, SHADER_DIR_PATH "bubble_geometry.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "bubble_fragment.glsl")})};

  auto skybox_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "skybox_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "skybox_fragment.glsl")})};

  auto mountain_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "mountain_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "mountain_fragment.glsl")})};

  auto drone_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "drone_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "drone_fragment.glsl")})};

  auto rotor_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "rotor_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "rotor_fragment.glsl")})};

  auto lava_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "lava_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "lava_fragment.glsl")})};

  auto health_bar_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "health_bar_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "health_bar_fragment.glsl")})};

  auto landing_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "landing_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "landing_fragment.glsl")})};
  auto landing_zone_shader{std::make_shared< glhelp::ShaderProgram >(std::vector{
      glhelp::create_shader_from_file(GL_VERTEX_SHADER, SHADER_DIR_PATH "landing_vertex.glsl"),
      glhelp::create_shader_from_file(GL_FRAGMENT_SHADER, SHADER_DIR_PATH "landing_zone_fragment.glsl")})};

  const glm::vec3 start_point{-450.0F, get_mountain_heigth(-450.0F, -450.0F), -450.0F};
  const glm::vec3 player_start{start_point + glm::vec3{0, 2.0F, 0}};

  // Object creation
  auto icosphere_obj{glhelp::Obj< glhelp::VertexNormals >::parse_from_file(reduce_geometry
                                                                               ? OBJ_DIR_PATH "simple_icosphere.obj"
                                                                               : OBJ_DIR_PATH "icosphere.obj")};
  auto helicopter_landing_obj{glhelp::Obj< glhelp::VertexNormals >::parse_from_file(OBJ_DIR_PATH "helicopter_landing.obj")};
  auto helicopter_zone_obj{glhelp::Obj< glhelp::VertexNormals >::parse_from_file(OBJ_DIR_PATH "helicopter_zone.obj")};

  auto cube_obj{glhelp::Obj< glhelp::SimpleVertex >::parse_from_file(OBJ_DIR_PATH "cube.obj")};

  auto drone_obj{glhelp::Obj< glhelp::VertexNormals >::parse_from_file(OBJ_DIR_PATH "dron.obj")};
  auto rotor_obj{glhelp::Obj< glhelp::VertexNormals >::parse_from_file(OBJ_DIR_PATH "rotor.obj")};

  while (window->is_active()) {
    auto drone{std::make_shared< Drone >(
        glhelp::FPSPlayerController(glhelp::FPSSimplePosition(player_start, glm::pi< float >(), 0, 0, glm::vec3{0.1f}), 5.0, 1),
        drone_shader,
        drone_obj,
        rotor_shader,
        rotor_obj,
        health_bar_shader,
        cube_obj)};

    using DroneFollower = glhelp::PositionFollower< DronePosition >;
    auto camera{std::make_shared< glhelp::Camera< DroneFollower > >(
        window,
        DroneFollower(
            *drone,
            glm::vec3{0.0F, 1.0F, 2.0F},
            glm::pi< float >(), 0, 0,
            glm::vec3{1.0F},
            glhelp::ADD_OFFSET_WITH_ROTATION,
            glhelp::PASSTHROUGH),
        90.0F, 0.1F, 10000.0F)};

    auto minimap_camera{std::make_shared< glhelp::Camera< glhelp::CachingSimplePosition > >(
        window,
        glhelp::CachingSimplePosition(
            glm::vec3{0.0F, 300.0F, 0.0F},
            0.0F, -glm::pi< float >() / 2.0F, 0,
            glm::vec3{1.0F}),
        50.0F, 0.1F, 10000.0F)};

    auto bubbles{std::make_shared< Bubbles >(
        bubble_shader,
        glhelp::SimplePosition(glm::vec3{0.0F, 0.0F, 0.0F}, 0, 0, 0, glm::vec3{1.0F}),
        icosphere_obj,
        get_bubble_data(n, rng))};

    auto mountain{std::make_shared< Mountain >(
        glhelp::SimplePosition{},
        mountain_shader,
        reduce_geometry ? 400 : 800)};

    using StaticMesh = glhelp::Mesh3D< glhelp::CachingSimplePosition >;
    auto landing_zone{std::make_shared< StaticMesh >(
        glhelp::CachingSimplePosition(start_point, 0, 0, 0, {1.0F, 1.0F, 1.0F}),
        landing_zone_shader,
        helicopter_zone_obj)};
    auto landing{std::make_shared< StaticMesh >(
        glhelp::CachingSimplePosition(start_point, 0, 0, 0, {1.0F, 1.0F, 1.0F}),
        landing_shader,
        helicopter_landing_obj)};

    auto skybox_position{glhelp::PositionFollower< DroneFollower >(
        *camera,
        glm::vec3{0.0},
        0, 0, 0,
        glm::vec3{1.0F, 1.0F, 1.0F} * 4900.0F,
        glhelp::PASSTHROUGH)};
    using Skybox = glhelp::Mesh3D< glhelp::PositionFollower< DroneFollower > >;
    auto skybox{std::make_shared< Skybox >(
        skybox_position,
        skybox_shader,
        cube_vertices,
        cube_indices_rev,
        GL_TRIANGLES)};

    auto lava{std::make_shared< StaticMesh >(
        glhelp::CachingSimplePosition{glm::vec3{0.0F, 225.0F, 0.0F}, 0, 0, 0, glm::vec3{7.0F, 1.0f, 7.0f}},
        lava_shader,
        icosphere_obj)};
    auto lava_light{glhelp::MovingSpotLight< glhelp::CachingSimplePosition >::create_shared(
        glhelp::CachingSimplePosition(
            lava->get_position(),
            0, glm::pi< float >() / 2.0F, 0,
            glm::vec3{1.0F}),
        glm::vec3{1.0F, 0.2F, 0.2F},
        0.0F, 0.0001F,
        glm::radians(90.0F))};

    auto sun{std::make_shared< glhelp::DirectionalLight >(glhelp::DirectionalLight{
        .direction = glm::normalize(glm::vec3{-1.0F, -1.0F, -1.0F}),
        .color = glm::vec3{1.1F, 1.1F, 1.0F},
    })};

    const auto flashlight_color{glm::vec3{1.0F, 1.0F, 0.6F}};
    auto flashlight_left{glhelp::MovingSpotLight< DroneFollower >::create_shared(
        DroneFollower(
            *drone,
            glm::vec3{0.2F, 0.0F, -0.4F},
            0, 0, 0,
            glm::vec3{1.0F},
            glhelp::ADD_OFFSET_WITH_ROTATION,
            glhelp::PASSTHROUGH),
        flashlight_color,
        0.0F, 0.001F,
        glm::radians(15.0F),
        glm::radians(30.0F))};
    auto flashlight_right{glhelp::MovingSpotLight< DroneFollower >::create_shared(
        DroneFollower(
            *drone,
            glm::vec3{-0.2F, 0.0F, -0.4F},
            0, 0, 0,
            glm::vec3{1.0F},
            glhelp::ADD_OFFSET_WITH_ROTATION,
            glhelp::PASSTHROUGH),
        flashlight_color,
        0.0F, 0.001F,
        glm::radians(15.0F),
        glm::radians(30.0F))};

    glhelp::Scene main_scene;

    main_scene.add_object(skybox);
    main_scene.add_object(mountain);
    main_scene.add_object(lava);
    main_scene.add_object(landing_zone);
    main_scene.add_object(landing);
    main_scene.add_object(drone);
    drone->add_to_scene(main_scene);

    main_scene.add_transparent_object(bubbles);

    main_scene.add_light(sun);
    main_scene.add_light(lava_light);
    main_scene.add_light(flashlight_left);
    main_scene.add_light(flashlight_right);

    drone->init_mouse(*window);

    glhelp::TogglingButton should_sort(GLFW_KEY_P, *window, true);
    auto sort_event{should_sort.event.connect([](bool sorting) {
      std::cerr << "Bubble sorting turned: " << (sorting ? "on\n" : "off\n");
    })};

    glhelp::TogglingButton drone_light(GLFW_KEY_L, *window, true);
    auto light_event{drone_light.event.connect([&](bool lights) {
      const float mult{lights ? 1.0F : 0.0F};
      flashlight_left->color = flashlight_color * mult;
      flashlight_right->color = flashlight_color * mult;
    })};

    auto scroll_event{window->scroll_event.connect([&](float, float yoffset) {
      camera->set_fov(glm::clamp(camera->get_fov() + yoffset, 55.0F, 120.0F));
    })};

    float minimap_scale{700.0F};
    glhelp::EventButton minimap_change(GLFW_KEY_TAB, *window);
    auto minimap_change_event{minimap_change.event.connect([&]() {
      minimap_change(); // Clear the event...
      minimap_scale -= 200.0F;
      if (minimap_scale < 100.0F) {
        minimap_scale = 700.0F;
      }
    })};

    window->run_synchronously([&, death_time = 0.0F]([[maybe_unused]] glhelp::Window& window, double time, double frame_time) mutable -> bool {
      const glm::vec3 old_position{drone->get_position()};
      const glm::vec3 old_rotation{drone->rotation};

      // Do not move the player if they are dead...
      if (!drone->health_bar.value()->is_alive()) {
        if (drone->disabled == false) {
          death_time = time;
          std::cerr << "Recorded player death; Disabling input.\n";
          std::cout << "Przegrałeś :( (na poziomie trudności " << n << ")\n";
        }
        drone->disabled = true;
        drone->position.y -= glm::pow((time - death_time), 2.0F) * frame_time;
      }
      else {
        drone->rotate_rotors(time);
      }

      drone->poll_keys(window, frame_time);

      auto collision_predicate{[&](glm::vec3 position, float radius) {
        return mountain->check_collision(position, radius);
      }};

      // Damage player if they go too far...
      if (glm::pow(drone->position.x, 2.0F) + glm::pow(drone->position.z, 2.0F) >= glm::pow(650.0F, 2.0F)) {
        drone->health_bar.value()->distance_damage(frame_time);
      }

      // Check collisions wit terrain (simple model).
      if (drone->check_terrain_collision(collision_predicate)) {
        drone->position = old_position + (drone->health_bar.value()->is_alive()
                                              ? (0.5F * glhelp::UP_VECTOR * static_cast< float >(frame_time))
                                              : glm::vec3{0.0F});
        drone->rotation = old_rotation;
      }

      // Update bubble possitions.
      bubbles->update_bubbles(drone->get_position(), time, frame_time, should_sort(), *(drone->health_bar.value()));

      // After all positions checks, check if we have arrived at the 'volcano core'.
      if (glm::distance(drone->get_position(), lava->get_position()) < 4.0F) {
        std::cout << "Ukonczono poziom z przy trudności " << n << " w czasie " << time << " sekund. Ładowanie następnego poziomu...\n";
        return false;
      }

      main_scene.draw_objects(*camera, time);
      main_scene.draw_minimap(*minimap_camera, window, time, minimap_scale, {0, 0}, {window.get_size().x * 0.2F, window.get_size().x * 0.2F});
      return true;
    });

    n += 10;
  }
}

static void print_help(const char* argv0)
{
  std::cerr << "Usage: " << argv0 << " [-n <number>] [-s <seed>] [-r]\n"
            << "  -n <number> : Number of bubbles to generate for first level (default: 20)\n"
            << "  -s <seed>   : Seed for random number generator\n"
            << "  -r          : Reduce geometry complexity for mountain and bubbles\n";
}

auto main(int argc, char* argv[]) -> int
{
  unsigned n{20};
  int seed{5}; // Uczciwy rzut kością!
  bool reduce_geometry{false};

  // 'paskudny-parser'
  for (int i{1}; i < argc; ++i) {
    if (argv[i] == std::string("-n") && i + 1 < argc) {
      n = std::stoi(argv[++i]);
    }
    else if (argv[i] == std::string("-s") && i + 1 < argc) {
      seed = std::stoi(argv[++i]);
    }
    else if (argv[i] == std::string("-h")) {
      print_help(argv[0]);
      return 0;
    }
    else if (argv[i] == std::string("-r")) {
      reduce_geometry = true;
    }
    else {
      std::cerr << "Unknown option: " << argv[i] << '\n';
      print_help(argv[0]);
      return 1;
    }
  }

  std::mt19937 rng(seed);

  try {
    glhelp::GLFWContext context;
    run_program(rng, n, reduce_geometry);
  }
  catch (std::exception& e) {
    std::cerr << "Program crashed!:\n"
              << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
