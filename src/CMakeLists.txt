cmake_minimum_required(VERSION 3.20)
project(Seamly2DSrc LANGUAGES CXX)
include(${CMAKE_SOURCE_DIR}/cmake/GlobHelpers.cmake)

add_subdirectory(libs)
add_subdirectory(app)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
