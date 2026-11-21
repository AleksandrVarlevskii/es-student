add_library(device STATIC
    src/device.c
)

target_include_directories(device PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(device PRIVATE
    pico_stdlib
    pico_unique_id
	logging
)

target_compile_definitions(device PRIVATE
    DEVICE_BOARD="${PICO_BOARD}"
)