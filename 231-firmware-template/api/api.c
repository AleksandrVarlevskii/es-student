#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "pico/stdlib.h"

#include "api.h"
#include "device/device.h"
#include "profiling/profiling.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"
#include "firmware.h"
#include "systime/systime.h"

// Прототипы — до таблицы
static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_led_period(const command_t *command);
static void cmd_led_enable(const command_t *command);
static void cmd_led_disable(const command_t *command);
static void cmd_led_blink(const command_t *command);
static void cmd_button(const command_t *command);
static void cmd_pi_start(const command_t *command);
static void cmd_pi(const command_t *command);
static void cmd_profiling(const command_t *command);
static void cmd_profiling_reset(const command_t *command);


const api_command_t api_commands[] = {
    { "info",            "device passport",                                   cmd_info            },
    { "uptime",          "time since reset",                                  cmd_uptime          },
    
    // LED
    { "led_enable",      "turn LED on",                                       cmd_led_enable      },
    { "led_disable",     "turn LED off",                                      cmd_led_disable     },
    { "led_blink",       "start blinking",                                    cmd_led_blink       },
    { "led_period",      "set blink period: led_period [ms]",                 cmd_led_period      },
    // BUTTON
    { "button",          "get button click count and reset: button",          cmd_button          },
    // PI
    { "pi_start",        "starts the calculation of pi and responds immediately, without waiting for the result.",   cmd_pi_start        },
    { "pi",       "displays the result of the last calculation or the calculation status if there is no result.",           cmd_pi       },
    //PROFILING
    { "profiling",       "print all stopwatches since reset",              cmd_profiling       },
    { "profiling_reset", "reset all stopwatches",                          cmd_profiling_reset },
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
    device_info_t info;
    device_get_info(&info);
    printf("name: %s\n", FIRMWARE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", FIRMWARE_PROJECT);
    printf("repo: %s\n", FIRMWARE_REPO);
    printf("board: %s\n", info.board);
    printf("serial: %s\n", info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", (unsigned)info.chip_manufacturer, (unsigned)info.chip_part, (unsigned)info.chip_revision);
    printf("pico-sdk: %s\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    uint64_t ms = systime_us() / 1000;
    printf("uptime: %llu ms\r\n", (unsigned long long)ms);
}


// LED


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



// BUTTON
static void cmd_button(const command_t *command)
{
    (void)command; // пояняем компилятору, что мы не используем аргументы команды
    printf("button: %s, presses %u\r\n", get_button_statename(), (unsigned)button_task_get_press_count());
    
}

// PI
static void cmd_pi_start(const command_t *command){

    uint32_t terms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &terms))
    {
        printf("error: usage pi_start <terms>\n");
        return;
    } else if (terms == 0) {
        printf("error: terms must be greater than 0 — <terms>\n");
        return;
    } else if (pi_task_get_state() == PI_TASK_STATE_RUNNING) {
        printf("error: pi is busy\n");
        return;
    }else if (!pi_task_start(terms)) {
        printf("error: pi calculation is already running\n");
        return;
    }
}







static void cmd_pi(const command_t *command){
    (void)command;
    pi_task_state_t state = pi_task_get_state();
    if (state == PI_TASK_STATE_IDLE){
        printf("error: pi: not started\n");
    }else if (state == PI_TASK_STATE_RUNNING){
        printf("pi: running\n");
    }else if (state == PI_TASK_STATE_READY){
        double result;
        uint64_t time_us;
        pi_task_get_result(&result, &time_us);
        printf("pi: %.10f in %llu ms\r\n", result, (unsigned long long)(time_us/1000)); // convert to milliseconds
    }

}

// PROFILING
#if PROFILING_ENABLED
static void cmd_profiling(const command_t *command)
{
    (void)command;
    profiling_result_t r;
    for (uint32_t id = 0; id < PROFILING_MAX_STOPWATCHES; id++) {
        if (!profiling_get(id, &r)) continue;

        // mean в profiling_result_t — целые микросекунды;
        // сотые доли считаем для печати из суммы и числа замеров
        uint32_t mean_centi = (r.number_measurements > 0)
            ? (uint32_t)(((uint64_t)r.sum_us * 100) / r.number_measurements)
            : 0;

        printf("%-10s count %-10u mean %u.%02u us  max %llu us\r\n",
               r.name,
               (unsigned)r.number_measurements,
               (unsigned)(mean_centi / 100),
               (unsigned)(mean_centi % 100),
               (unsigned long long)r.max);
    }
}
#else
static void cmd_profiling(const command_t *command){
    (void)command;
    printf("profiling is disabled in this build\n");
}
#endif

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
}