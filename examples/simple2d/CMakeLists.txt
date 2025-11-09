file(GLOB_RECURSE EXAMPLE_SOURCES "src/*.cpp")

add_executable(example_simple ${EXAMPLE_SOURCES})

target_link_libraries(example_simple PRIVATE glhelp)

target_include_directories(example_simple
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

add_custom_command(
  TARGET example_simple POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "${CMAKE_CURRENT_BINARY_DIR}/shaders"
  COMMENT "Copying resources (shaders) to build directory"
)
