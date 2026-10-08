
#pragma once

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.0.0"

#define DEVICE_PROJECT "134-led-module"
#define DEVICE_REPO "https://github.com/AleksandrVarlevskii/es-student"
#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

// Размер строки для уникального ID платы (8 байт ID -> 16 hex-символов + '\0')
#define BOARD_ID_STR_LEN 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1


void device_info(void);
