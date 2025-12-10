cmake_minimum_required(VERSION 4.2)

include(../common/hw_impl.cmake)
include(../common/util.cmake)
include(../common/sprite.cmake)

add_executable(angkor_grass angkor_grass.c)
target_link_libraries(angkor_grass sprite util hw_impl)
