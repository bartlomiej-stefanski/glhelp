file(GLOB_RECURSE EXAMPLE_SOURCES "src/*.cpp")

add_executable(labirynt3d ${EXAMPLE_SOURCES})

target_link_libraries(labirynt3d PRIVATE glhelp)

target_include_directories(labirynt3d
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

target_compile_definitions(labirynt3d PRIVATE
  SHADER_DIR_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
)

add_custom_command(
  TARGET labirynt3d POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "${CMAKE_CURRENT_BINARY_DIR}/shaders"
  COMMENT "Copying resources (shaders) to build directory"
)
