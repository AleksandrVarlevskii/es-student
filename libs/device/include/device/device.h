#pragma once

#include <stdint.h>


#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif
// Размер строки для уникального ID платы (8 байт ID -> 16 hex-символов + '\0')
// при подлкючении pico/unique_id.h это 17 может быть вычислина  2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1
#define BOARD_ID_STR_LEN 17 

typedef struct {
    const char *board;              // тип платы (DEVICE_BOARD)
    char        serial[BOARD_ID_STR_LEN]; // серийный номер (hex-строка)
    uint32_t    chip_manufacturer;  // производитель кристалла
    uint32_t    chip_part;          // изделие (part)
    uint32_t    chip_revision;      // ревизия кристалла
    const char *sdk_version;        // версия SDK
} device_info_t;

// Заполняет структуру по указателю
void device_get_info(device_info_t *info);

extern struct info_t device_card;




