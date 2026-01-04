file(GLOB_RECURSE EXAMPLE_SOURCES "src/*.cpp")

add_subdirectory(hgt)

add_executable(mapa ${EXAMPLE_SOURCES})

target_link_libraries(mapa PRIVATE glhelp hgt)

target_include_directories(mapa
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

target_compile_definitions(mapa PRIVATE
  SHADER_DIR_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
  OBJ_DIR_PATH="${CMAKE_CURRENT_SOURCE_DIR}/models/"
)

add_custom_command(
  TARGET mapa POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "${CMAKE_CURRENT_BINARY_DIR}/shaders"
  COMMENT "Copying resources (shaders) to build directory"
)
