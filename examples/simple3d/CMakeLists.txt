file(GLOB_RECURSE EXAMPLE_SOURCES "src/*.cpp")

add_executable(simple3d ${EXAMPLE_SOURCES})

target_link_libraries(simple3d PRIVATE glhelp)

target_include_directories(simple3d
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

add_custom_command(
  TARGET simple3d POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "${CMAKE_CURRENT_BINARY_DIR}/shaders"
  COMMENT "Copying resources (shaders) to build directory"
)
