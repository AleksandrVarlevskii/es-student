

#include "api-commands.h"
#include "device/device.h"
#include "profiling/profiling.h"
#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"
#include "firmware.h"
#include "systime/systime.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>


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

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);



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



// --- обработчики ---

static void cmd_info(const command_t *command)
{
    (void)command;
    device_info_t info;
    device_get_info(&info);
    printf("name: %s\r\n", FIRMWARE_NAME);
    printf("version: %s\r\n", FIRMWARE_VERSION);
    printf("project: %s\r\n", FIRMWARE_PROJECT);
    printf("repo: %s\r\n", FIRMWARE_REPO);
    printf("board: %s\r\n", info.board);
    printf("serial: %s\r\n", info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\r\n", (unsigned)info.chip_manufacturer, (unsigned)info.chip_part, (unsigned)info.chip_revision);
    printf("pico-sdk: %s\r\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    uint64_t ms = systime_us() / 1000;
    printf("uptime: %llu ms\r\n", (unsigned long long)ms);
}


// LED

static void print_led_state(void){
    const char *name_state;
    switch (led_task_get_state())
    {
    case LED_STATE_OFF:
        name_state = "off";
        break;
    case LED_STATE_ON:
        name_state = "on";
        break;
    default:
        name_state = "blink";
        break;
    }
    printf("led: %s, period %lu ms \r\n", name_state, led_task_get_period_ms());
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\r\n");
        return;
    }

    // годится ли такой период, решает задача светодиода
    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\r\n");
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

        // Значение по умолчанию
    uint32_t terms = 1000000;

    // 1. Слишком много аргументов?
    if (command->argc > 1) {
        printf("error: usage pi_start [<terms>]\r\n");
        return;
    }

    // 2. Аргумент есть? Тогда пробуем его распарсить
    if (command->argc == 1) {
        if (!parse_u32(command->argv[0], &terms)) {
            printf("error: usage pi_start [<terms>]\r\n");
            return;
        }
        if (terms == 0) {
            printf("error: terms must be greater than 0\r\n");
            return;
        }
    }
    
    // Если argc == 0, мы просто пропускаем этот блок и используем terms = 1000000

    // 3. Проверяем состояние задачи
    if (pi_task_get_state() == PI_TASK_STATE_RUNNING) {
        printf("error: pi is busy\r\n");
        return;
    }

    // 4. Запускаем расчет
    if (!pi_task_start(terms)) {
        printf("error: pi calculation failed to start\r\n");
        return;
    }

    printf("pi: started, %u terms\r\n", (unsigned)terms);
}







static void cmd_pi(const command_t *command){
    (void)command;
    pi_task_state_t state = pi_task_get_state();
    if (state == PI_TASK_STATE_IDLE){
        printf("pi: not started\r\n");
    }else if (state == PI_TASK_STATE_RUNNING){
        printf("pi: running\r\n");
    }else if (state == PI_TASK_STATE_READY){
        double result;
        uint64_t time_us;
        pi_task_get_result(&result, &time_us);
        printf("pi: %.9f in %llu ms\r\n", result, (unsigned long long)(time_us/1000)); // convert to milliseconds
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

        printf("%-10s count %-10u mean %u.%02u us max %llu us\r\n",
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
    printf("profiling is disabled in this build\r\n");
}
#endif

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
    printf("profiling reset\r\n");
}