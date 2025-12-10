add_library(sprite STATIC
    ${CMAKE_CURRENT_LIST_DIR}/sprite/palette.c
    ${CMAKE_CURRENT_LIST_DIR}/sprite/sprite.c
    ${CMAKE_CURRENT_LIST_DIR}/sprite/texture.c
)

target_include_directories(sprite PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/sprite
)

target_link_libraries(sprite PRIVATE
    hw_impl
)
