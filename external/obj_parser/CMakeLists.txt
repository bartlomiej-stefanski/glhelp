cmake_minimum_required(VERSION 3.24)

find_package(glm 0.9.8 REQUIRED)

file(GLOB_RECURSE OBJ_LOADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(obj_parser STATIC ${OBJ_LOADER_SOURCES})

target_link_libraries(obj_parser
  PUBLIC
    glm
)

target_include_directories(obj_parser
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
