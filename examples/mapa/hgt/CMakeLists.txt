cmake_minimum_required(VERSION 3.24)

find_package(Threads REQUIRED)
find_package(glm 0.9.8 REQUIRED)

file(GLOB_RECURSE OBJ_LOADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(hgt STATIC ${OBJ_LOADER_SOURCES})

target_link_libraries(hgt
  PUBLIC
    glm::glm
)

target_include_directories(hgt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
