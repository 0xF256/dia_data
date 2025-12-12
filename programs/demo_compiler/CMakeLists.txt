cmake_minimum_required(VERSION 4.2)

project(demo_compiler)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(demo_parser demo_parser.c)
add_executable(demo_compiler demo_compiler.c)
