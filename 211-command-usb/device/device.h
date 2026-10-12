#pragma once

#include <stdint.h>

#define DEVICE_NAME "es-cmd-usb"
#define FIRMWARE_VERSION "1.0.0"

#define DEVICE_PROJECT "211-command-usb"
#define DEVICE_REPO "https://github.com/AleksandrVarlevskii/es-student"
#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

struct info_t
{
    uint8_t revision;
    uint32_t version;
    char name[13];
};

extern struct info_t device_card;


// Размер строки для уникального ID платы (8 байт ID -> 16 hex-символов + '\0')
#define BOARD_ID_STR_LEN 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1
void device_card_init(void);

void dev_info(void);

void device_info(void);


