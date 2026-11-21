add_library(systime STATIC
    src/systime.c
)

target_include_directories(systime PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)



target_link_libraries(systime PRIVATE
	pico_time
	)