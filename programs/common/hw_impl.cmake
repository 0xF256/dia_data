add_library(hw_impl STATIC
    ${CMAKE_CURRENT_LIST_DIR}/hw/engine_tex_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/hw/file_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/hw/graphic.c
)

target_include_directories(hw_impl PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/hw
)

target_link_libraries(hw_impl PRIVATE
    SDL2
)
