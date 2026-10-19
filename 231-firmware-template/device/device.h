#pragma once

#include <stdint.h>

#define DEVICE_NAME "pico"
#define FIRMWARE_VERSION "1.2.0"

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/AleksandrVarlevskii/es-student"
#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

struct info_t
{
    uint32_t version;
    char name[13];
    uint8_t revision;
    
   
};

extern struct info_t device_card;


// Размер строки для уникального ID платы (8 байт ID -> 16 hex-символов + '\0')
#define BOARD_ID_STR_LEN 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1

void dev_info(void);

void device_info(void);


