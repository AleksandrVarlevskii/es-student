#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"
#include "memory.h"
#include "command.h"
#include "device.h"
#include <stdlib.h>
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

int main(void);

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


uint32_t data_variable = 100;
uint32_t bss_variable;

static void print_func(const char *name, void (*fn)(void))
{
    uintptr_t addr = (uintptr_t)fn;
    uint16_t  code = *(const uint16_t *)(addr & ~1u);
    printf("%-16s 0x%08x  0x%04x\n", name, (unsigned)addr, code);
}

void fw_info(void)
{
    uint32_t  stack_variable = 1946;
    uint32_t *heap_variable  = malloc(sizeof(uint32_t));
    *heap_variable = 1951;

    printf("object          address     value\n");

    print_func("main",    (void (*)(void))main);
    print_func("fw_info", fw_info);

    printf("%-16s 0x%08x\n", "commands",
           (unsigned)(uintptr_t)commands);

    for (size_t i = 0; i < command_count; i++) {
        char label[32];
        snprintf(label, sizeof(label), "- %s", commands[i].name);
        printf("%-16s 0x%08x\n", label,
               (unsigned)(uintptr_t)commands[i].handler);
    }

    printf("%-16s 0x%08x  %s\n", "DEVICE_PROJECT",
           (unsigned)(uintptr_t)DEVICE_PROJECT, DEVICE_PROJECT);

    printf("%-16s 0x%08x  %s\n", "DEVICE_BOARD",
           (unsigned)(uintptr_t)DEVICE_BOARD, DEVICE_BOARD);

    printf("%-16s 0x%08x  %u\n", "data_variable",
           (unsigned)(uintptr_t)&data_variable, data_variable);

    printf("%-16s 0x%08x  %u\n", "bss_variable",
           (unsigned)(uintptr_t)&bss_variable, bss_variable);

    printf("%-16s 0x%08x  %u\n", "stack_variable",
           (unsigned)(uintptr_t)&stack_variable, stack_variable);

    printf("%-16s 0x%08x  %u\n", "heap_variable",
           (unsigned)(uintptr_t)heap_variable, *heap_variable);

    free(heap_variable);
}

