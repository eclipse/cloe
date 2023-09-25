cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(LRDB LANGUAGES CXX)

add_library(lrdb INTERFACE)
add_library(lrdb::lrdb ALIAS lrdb)
target_include_directories(lrdb
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/picojson>"
)
target_link_libraries(lrdb
  INTERFACE
    lua::lua
)
set_target_properties(lrdb PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)
