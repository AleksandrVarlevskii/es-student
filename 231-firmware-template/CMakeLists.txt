cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(231_firmware_template)

pico_sdk_init()

add_compile_options(-Wall)

# ==========================================
# 1. Общие библиотеки (libs/ вне проекта)
# ==========================================
add_subdirectory(../libs ${CMAKE_BINARY_DIR}/libs)

# выключатель профилирования
target_compile_definitions(profiling PUBLIC
    PROFILING_ENABLED=1
)

# ==========================================
# 2. platform — железо прибора 
# ==========================================
add_library(platform STATIC
    platform/platform.c
)

# PUBLIC нужно, чтобы logic тоже могла найти firmware.h через platform
target_include_directories(platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(platform PRIVATE
    pico_stdlib
    pico_unique_id
)

# ==========================================
# 3. logic — задачи и команды
# ==========================================
add_library(logic STATIC
    logic/api/api.c
    logic/api/api-callbacks.c
    logic/led-task/led-task.c
    logic/button-task/button-task.c
    logic/pi-task/pi-task.c
)

# Путь к заголовкам — папка logic
target_include_directories(logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
)


target_link_libraries(logic PUBLIC
    stdio_text_protocol
    platform
)

# Остальные библиотеки — PRIVATE (нужны только внутри .c файлов logic)
target_link_libraries(logic PRIVATE
    profiling
    systime
    logging
    device
)

# ==========================================
# 4. Исполняемый файл (прошивка)
# ==========================================
add_executable(${PROJECT_NAME}
    main.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    logic
    platform
    pico_stdlib
	logging
    device
    profiling
    stdio_text_protocol
)

pico_set_linker_script(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/memmap_rp2040.ld
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})