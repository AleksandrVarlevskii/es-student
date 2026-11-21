add_library(stdio_text_protocol STATIC
    src/stdio-text-protocol.c
)

target_include_directories(stdio_text_protocol PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)



target_link_libraries(stdio_text_protocol PRIVATE
	logging
	pico_stdlib # Протоколу нужен стандартный ввод-вывод SDK. Подключайте ему pico_stdlib целиком: pico_stdio без него не собирается.
)