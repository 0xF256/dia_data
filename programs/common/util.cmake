add_library(util STATIC
    ${CMAKE_CURRENT_LIST_DIR}/util/chunk.c
)

target_include_directories(util PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/util
)

target_link_libraries(util PRIVATE
    hw_impl
)
