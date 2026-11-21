#include "device.h"

#include <stddef.h>
#include <stdio.h>

#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include "pico/version.h"
#include "log.h"




void device_info(void){
    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;

    uint32_t manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    uint32_t part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    uint32_t revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;

    char board_id[BOARD_ID_STR_LEN];
    pico_get_unique_board_id_string(board_id, sizeof(board_id));

    printf("name: %s\n", DEVICE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", DEVICE_PROJECT);
    printf("repo: %s\n", DEVICE_REPO);
    printf("board: %s\n", DEVICE_BOARD);
    printf("serial: %s\n", board_id);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", (unsigned)manufacturer, (unsigned)part, (unsigned)revision);
    printf("pico-sdk: %s\n", PICO_SDK_VERSION_STRING);
}


void device_get_info(device_info_t *info) {
    if (info == NULL){
        LOG_ERR("device_get_info: info is NULL");
        return;
    }

    // 1. Константы прошивки
    info->name    = DEVICE_NAME;
    info->version = FIRMWARE_VERSION;
    info->project = DEVICE_PROJECT;
    info->repo    = DEVICE_REPO;
    info->board   = DEVICE_BOARD;

    // 2. Серийный номер
    pico_get_unique_board_id_string(info->serial, sizeof(info->serial));

    // 3. Данные кристалла из SYSINFO
    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;

    info->chip_manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    info->chip_part         = (id & SYSINFO_CHIP_ID_PART_BITS)         >> SYSINFO_CHIP_ID_PART_LSB;
    info->chip_revision     = (id & SYSINFO_CHIP_ID_REVISION_BITS)     >> SYSINFO_CHIP_ID_REVISION_LSB;

    // 4. Версия SDK
    info->sdk_version = PICO_SDK_VERSION_STRING;
}