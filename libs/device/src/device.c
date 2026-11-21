

#include <stddef.h>
#include <stdio.h>

#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include "pico/version.h"

#include "device/device.h"
#include "logging/log.h"




void device_get_info(device_info_t *info) {
    if (info == NULL){
        LOG_ERR("device_get_info: info is NULL");
        return;
    }

    // 1. Константы прошивки
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