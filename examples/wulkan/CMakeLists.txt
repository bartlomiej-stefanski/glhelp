file(GLOB_RECURSE EXAMPLE_SOURCES "src/*.cpp")

add_executable(wulkan ${EXAMPLE_SOURCES})

target_link_libraries(wulkan PRIVATE glhelp)

target_include_directories(wulkan
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

target_compile_definitions(wulkan PRIVATE
  SHADER_DIR_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
)

add_custom_command(
  TARGET wulkan POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "${CMAKE_CURRENT_BINARY_DIR}/shaders"
  COMMENT "Copying resources (shaders) to build directory"
)
