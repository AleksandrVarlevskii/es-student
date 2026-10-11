#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"
#include "memory.h"

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

#define SRAM_SIZE_BYTES (264u * 1024u)
#define ROM_SIZE_BYTES  ( 16u * 1024u)

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    printf("area       start      end        size\n");

    row("flash",
        (uintptr_t)XIP_BASE,
        (uintptr_t)XIP_BASE + PICO_FLASH_SIZE_BYTES);

    row("sram",
        (uintptr_t)SRAM_BASE,
        (uintptr_t)SRAM_BASE + SRAM_SIZE_BYTES);

    row("rom",
        (uintptr_t)ROM_BASE,
        (uintptr_t)ROM_BASE + ROM_SIZE_BYTES);

    row("image",
        (uintptr_t)&__flash_binary_start,
        (uintptr_t)&__flash_binary_end);

    row("free",
        (uintptr_t)&__flash_binary_end,
        (uintptr_t)XIP_BASE + PICO_FLASH_SIZE_BYTES);

    row("boot2",
        (uintptr_t)&__boot2_start__,
        (uintptr_t)&__boot2_end__);

    row("text",
        (uintptr_t)&__boot2_end__,
        (uintptr_t)&__etext);

    row("data flash",
        (uintptr_t)&__etext,
        (uintptr_t)&__etext
            + ((uintptr_t)&__data_end__ - (uintptr_t)&__data_start__));

    row("data ram",
        (uintptr_t)&__data_start__,
        (uintptr_t)&__data_end__);

    row("bss",
        (uintptr_t)&__bss_start__,
        (uintptr_t)&__bss_end__);

    row("heap",
        (uintptr_t)&__bss_end__,
        (uintptr_t)&__HeapLimit);

    row("stack",
        (uintptr_t)&__StackBottom,
        (uintptr_t)&__StackTop);

    const uintptr_t image_sz  = (uintptr_t)&__flash_binary_end
                              - (uintptr_t)&__flash_binary_start;
    const uintptr_t boot2_sz  = (uintptr_t)&__boot2_end__
                              - (uintptr_t)&__boot2_start__;
    const uintptr_t text_sz   = (uintptr_t)&__etext
                              - (uintptr_t)&__boot2_end__;
    const uintptr_t dataf_sz  = (uintptr_t)&__data_end__
                              - (uintptr_t)&__data_start__;
    const uintptr_t datar_sz  = (uintptr_t)&__data_end__
                              - (uintptr_t)&__data_start__;
    const uintptr_t bss_sz    = (uintptr_t)&__bss_end__
                              - (uintptr_t)&__bss_start__;
    const uintptr_t heap_sz   = (uintptr_t)&__HeapLimit
                              - (uintptr_t)&__bss_end__;
    const uintptr_t stack_sz  = (uintptr_t)&__StackTop
                              - (uintptr_t)&__StackBottom;

    printf("\ntotal\n");

    printf("  flash image    %8u = boot2 %u + text %u + data %u\n",
           (unsigned)image_sz,
           (unsigned)boot2_sz,
           (unsigned)text_sz,
           (unsigned)dataf_sz);

    printf("  flash free     %8u of %u\n",
           (unsigned)(PICO_FLASH_SIZE_BYTES - image_sz),
           (unsigned)PICO_FLASH_SIZE_BYTES);

    printf("  ram used       %8u = data %u + bss %u\n",
           (unsigned)(datar_sz + bss_sz),
           (unsigned)datar_sz,
           (unsigned)bss_sz);

    printf("  ram free       %8u for heap and %u for stack\n",
           (unsigned)heap_sz,
           (unsigned)stack_sz);
}