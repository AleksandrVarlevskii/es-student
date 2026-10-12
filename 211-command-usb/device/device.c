#include "device.h"

#include <stddef.h>
#include <stdio.h>

#include "pico/unique_id.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include "pico/version.h"


struct info_t device_card = {
    .version  = 0x10007670u,
    .name     = "es-cmd-usb",
    .revision = 2,          
};

void device_card_init(void)
{
    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;

    device_card.version = (uint32_t)FIRMWARE_VERSION; 
    snprintf(device_card.name, sizeof(device_card.name), "%s", DEVICE_NAME);
    device_card.revision = (uint8_t)(
        (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB
    );

    
    
}

void dev_info(void){
    size_t fields = sizeof(device_card.version) + sizeof(device_card.name) + sizeof(device_card.revision);
    size_t size_device_card = sizeof(device_card);


    printf("%-15s %-10s %6s %6s %s\n",
            "struct", "address", "size", "offset", "value");

    printf("%-15s 0x%08x %5zu\n",
            "device_card",
            (unsigned)(uintptr_t)&device_card,
            size_device_card);
    printf("- %-13s 0x%08x %5zu %6zu 0x%08x\n",
            "version",
            (unsigned)(uintptr_t)&device_card.version,
            sizeof(device_card.version),
            offsetof(struct info_t, version),
            (unsigned)device_card.version);
    printf("- %-13s 0x%08x %5zu %6zu %s\n",
            "name",
            (unsigned)(uintptr_t)device_card.name,
            sizeof(device_card.name),
            offsetof(struct info_t, name),
            device_card.name);

    printf("- %-13s 0x%08x %5zu %6zu %u\n",
            "revision",
            (unsigned)(uintptr_t)&device_card.revision,
            sizeof(device_card.revision),
            offsetof(struct info_t, revision),
            (unsigned)device_card.revision);
    
    
    
    printf("fields %zu, sizeof %zu, padding %zu\n", fields, size_device_card , size_device_card-fields );
}





void device_info(void){
    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;

    uint32_t manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    uint32_t part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    uint32_t revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;

    char board_id[BOARD_ID_STR_LEN];
    pico_get_unique_board_id_string(board_id, sizeof(board_id));

    printf("project: %s\n", DEVICE_PROJECT);
    printf("repo: %s\n", DEVICE_REPO);
    printf("board: %s\n", DEVICE_BOARD);
    printf("serial: %s\n", board_id);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", manufacturer, part, revision);
    printf("pico-sdk: %s\n", PICO_SDK_VERSION_STRING);
}