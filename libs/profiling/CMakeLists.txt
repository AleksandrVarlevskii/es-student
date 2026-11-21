add_library(profiling STATIC
    src/profiling.c
)

target_include_directories(profiling PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(profiling PUBLIC
    pico_base_headers
)

target_link_libraries(profiling PRIVATE
    	pico_time
	logging
	systime
)