cmake_minimum_required(VERSION 4.2)

project(angkor_grass)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(../common/hw_impl.cmake)
include(../common/util.cmake)
include(../common/sprite.cmake)

add_executable(angkor_grass angkor_grass.c graphic.c)

if(EMSCRIPTEN)
    set_target_properties(angkor_grass PROPERTIES LINK_FLAGS "-sALLOW_MEMORY_GROWTH=1 --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/0.f@0.f")
endif()

target_link_libraries(angkor_grass sprite util hw_impl)
