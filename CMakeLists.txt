cmake_minimum_required(VERSION 3.24)

project(glhelp VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm 0.9.8 REQUIRED)

add_subdirectory(external/glad)

add_compile_options(-Wall -Wextra)

option(WARNINGS_AS_ERRORS "Treat compilation warnings as errors" OFF)
if(WARNINGS_AS_ERRORS)
  if(MSVC)
    add_compile_options(/WX)
  else()
    add_compile_options(-Werror)
  endif()
endif()

option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXECUTABLE clang-tidy QUIET)

  # Enable automatic fix application with -fix flag
  set(CLANG_TIDY_EXECUTABLE "${CLANG_TIDY_EXECUTABLE};-fix")

  set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE})
  message(STATUS "clang-tidy enabled. Using: ${CLANG_TIDY_EXECUTABLE}")
endif()

option(ENABLE_IWYU "Enable include-what-you-use analysis" OFF)
if(ENABLE_IWYU)
  find_program(IWYU_EXECUTABLE include-what-you-use REQUIRED)

  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXECUTABLE})
  message(STATUS "include-what-you-use enabled. Using: ${IWYU_EXECUTABLE}")
endif()

find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(Doxygen_FOUND)
  message(STATUS "Doxygen found, 'docs' target available.")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
  )

  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating Doxygen documentation..."
    VERBATIM
  )
endif()

add_subdirectory(external/obj_parser)

file(GLOB_RECURSE GLHELP_SOURCES "src/*.cpp")

add_library(glhelp ${GLHELP_SOURCES})

target_include_directories(glhelp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(glhelp
  PUBLIC
    glad_loader
    glfw
    glm
)

option(GLHELP_BUILD_EXAMPLES "Build the glhelp examples" ON)

if(GLHELP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
