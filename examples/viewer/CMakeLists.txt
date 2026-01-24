file(GLOB_RECURSE EXAMPLE_SOURCES "src/*.cpp")

add_executable(viewer ${EXAMPLE_SOURCES})

target_link_libraries(viewer PRIVATE glhelp)

target_include_directories(viewer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

target_compile_definitions(viewer PRIVATE
  SHADER_DIR_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
  OBJ_DIR_PATH="${CMAKE_CURRENT_SOURCE_DIR}/models/"
)

add_custom_command(
  TARGET viewer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "${CMAKE_CURRENT_BINARY_DIR}/shaders"
  COMMENT "Copying resources (shaders) to build directory"
)
