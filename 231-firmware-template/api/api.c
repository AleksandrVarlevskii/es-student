#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "pico/stdlib.h"

#include "api.h"
#include "device.h"
#include "profiling.h"
#include "led-task.h"
#include "button-task.h"

// Прототипы — до таблицы
static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_calc_pi(const command_t *command);
static void cmd_main_time_exec(const command_t *command);
static void cmd_main_time_reset(const command_t *command);
static void cmd_led_period(const command_t *command);
static void cmd_led_enable(const command_t *command);
static void cmd_led_disable(const command_t *command);
static void cmd_led_blink(const command_t *command);
static void cmd_button(const command_t *command);


const api_command_t api_commands[] = {
    { "info",            "device passport",                                   cmd_info            },
    { "uptime",          "time since reset",                                  cmd_uptime          },
    { "calc_pi",         "calculate pi: calc_pi [terms]",                     cmd_calc_pi         },
    { "main_time_exec",  "superloop iteration: average and maximum",          cmd_main_time_exec  },
    { "main_time_reset", "reset maximum iteration",                           cmd_main_time_reset },
    
    // LED
    { "led_enable",      "turn LED on",                                       cmd_led_enable      },
    { "led_disable",     "turn LED off",                                      cmd_led_disable     },
    { "led_blink",       "start blinking",                                    cmd_led_blink       },
    { "led_period",      "set blink period: led_period [ms]",                 cmd_led_period      },
    // BUTTON
    { "button",          "get button click count and reset: button",          cmd_button          },
};

static const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);

static bool parse_u32(const char *text, uint32_t *value)
{
    if (text == NULL || *text == '\0') {
        return false;
    }
    if (!isdigit((unsigned char)text[0])) {
        return false;
    }

    char *end = NULL;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX) {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}

void api_handle(const command_t *command)
{
    if (command == NULL || command->name == NULL) {
        return;
    }

    if (command->truncated) {
        printf("error: command truncated (max 63 chars, max 4 args)\r\n");
        return;
    }

    // help — вне таблицы: печатает все команды со справкой
    if (strcmp(command->name, "help") == 0) {
        printf("%-18s %s\r\n", "command", "description");
        for (uint32_t i = 0; i < api_command_count; i++) {
            printf("%-18s %s\r\n", api_commands[i].name, api_commands[i].help);
        }
        return;
    }

    for (uint32_t i = 0; i < api_command_count; i++) {
        if (strcmp(command->name, api_commands[i].name) == 0) {
            api_commands[i].callback(command);
            return;
        }
    }

    printf("error: unknown command '%s', try help\r\n", command->name);
}

// --- обработчики ---

static void cmd_info(const command_t *command)
{
    (void)command;
    device_info();
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    uint64_t ms = time_us_64() / 1000;
    printf("uptime: %llu ms\r\n", (unsigned long long)ms);
}

#define CALC_PI_DEFAULT_TERMS 1000000u

static double calc_pi(uint32_t terms)
{
    double sum  = 0.0;
    double sign = 1.0;

    for (uint32_t k = 0; k < terms; k++) {
        sum += sign / (2.0 * (double)k + 1.0);
        sign = -sign;
    }

    return 4.0 * sum;
}

static void cmd_calc_pi(const command_t *command)
{
    uint32_t terms = CALC_PI_DEFAULT_TERMS;

    if (command->argc > 1) {
        printf("error: usage calc_pi [terms]\r\n");
        return;
    }

    if (command->argc == 1) {
        if (!parse_u32(command->argv[0], &terms)) {
            printf("error: usage calc_pi [terms]\r\n");
            return;
        }
        if (terms == 0) {
            printf("error: terms must be greater than 0\r\n");
            return;
        }
    }

    uint64_t start_us = time_us_64();
    double   pi       = calc_pi(terms);
    uint64_t spent_us = time_us_64() - start_us;

    printf("pi: %.9f\r\n", pi);
    printf("time: %llu ms\r\n", (unsigned long long)(spent_us / 1000));
}

static void cmd_main_time_exec(const command_t *command)
{
    (void)command;
    printf("iteration avg %.2f us, max %u us\r\n",
           profiling_avg_us(),
           (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t *command)
{
    (void)command;
    profiling_reset_max();
    printf("max reset\r\n");
}


static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    // годится ли такой период, решает задача светодиода
    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    // ответ: состояние и период светодиода
    print_led_state();
}

static void cmd_led_enable(const command_t *command)
{
    led_task_set_state(LED_STATE_ON);
    // ответ: состояние и период светодиода
    print_led_state();
}

static void cmd_led_disable(const command_t *command)
{
    led_task_set_state(LED_STATE_OFF);
    // ответ: состояние и период светодиода
    print_led_state();
}

static void cmd_led_blink(const command_t *command)
{
    led_task_set_state(LED_STATE_BLINK);
    // ответ: состояние и период светодиода
    print_led_state();
}

static void cmd_button(const command_t *command)
{
    (void)command; // пояняем компилятору, что мы не используем аргументы команды
    printf("button: %s, presses %u\r\n", get_button_statename(), (unsigned)button_task_get_press_count());
    
}