
#include "led-task/led-task.h"
#include "logging/log.h"
#include "device/device.h"
#include "profiling/profiling.h"
#include "api/api.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"
#include "firmware.h"
#include "platform/platform.h"



enum {
    STOPWATCH_LOOP = 0,
    STOPWATCH_COMMAND,
    STOPWATCH_LED,
    STOPWATCH_BUTTON,
    STOPWATCH_PI,
    STOPWATCH_COUNT
};

static void profiling_init_all(void)
{
    profiling_stopwatch_init(STOPWATCH_LOOP,    "loop");
    profiling_stopwatch_init(STOPWATCH_COMMAND, "command");
    profiling_stopwatch_init(STOPWATCH_LED,     "led");
    profiling_stopwatch_init(STOPWATCH_BUTTON,  "button");
    profiling_stopwatch_init(STOPWATCH_PI,      "pi");
}








int main(){
    LOG_INF("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);
    platform_init();


    led_task_init();
    button_task_callback_t button_task_callback[BUTTON_COUNT] = {led_task_next_state, led_task_next_period};
    button_task_init(button_task_callback);
    stdio_text_protocol_init();
    pi_task_init();
    profiling_init_all();


    

    while (1)
    {
        profiling_start(STOPWATCH_LOOP);

        profiling_start(STOPWATCH_LED);
        led_task_handle();
        profiling_stop(STOPWATCH_LED);
        
        profiling_start(STOPWATCH_BUTTON);
        button_task_handle();
        profiling_stop(STOPWATCH_BUTTON);
        
        profiling_start(STOPWATCH_PI);
        pi_task_handle();
        profiling_stop(STOPWATCH_PI);

        profiling_start(STOPWATCH_COMMAND);
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }
        profiling_stop(STOPWATCH_COMMAND);
        

        profiling_stop(STOPWATCH_LOOP);
    }
}