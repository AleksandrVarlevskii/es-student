
#include "led-task/led-task.h"
#include "logging/log.h"
#include "systime/systime.h"
#include "platform/platform.h"



static led_state_t led_state;
static uint32_t half_period_ms = 500; // милисекунды 
static uint64_t last_toggle_us = 0;






void led_task_init(void)
{
    led_task_set_state(LED_STATE_BLINK);
    // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    switch (led_state)
    {
        case LED_STATE_OFF:
        platform_led_set(false);
        break;
        case LED_STATE_ON:
        platform_led_set(true);
        break;
        case LED_STATE_BLINK:
        platform_led_set(!platform_led_get());
        break;
        default:
        LOG_DBG("An led was made to establish a non-existent state of the led");
        break;
    }
}
led_state_t led_task_get_state(void){
    return led_state;
}



void led_task_handle(void)
{
    switch (led_state)
    {
        case LED_STATE_OFF:
        break;
        case LED_STATE_ON:
        break;
        case LED_STATE_BLINK:
        
        if (systime_period_elapsed(&last_toggle_us, half_period_ms*1000))
        {
        platform_led_set(!platform_led_get());
        }
        break;
    }
}

bool led_task_set_period_ms(uint32_t period_ms){
    if (period_ms == 0) {
        return false;
    }

    half_period_ms = period_ms / 2;
    return true;
}
uint32_t led_task_get_period_ms(void){
    return half_period_ms*2;
}


void led_task_next_state(void){
    switch (led_state){
        case LED_STATE_OFF:
            led_task_set_state(LED_STATE_ON);
            break;
        case LED_STATE_ON:
            led_task_set_state(LED_STATE_BLINK);
            break;
        case LED_STATE_BLINK:
            led_task_set_state(LED_STATE_OFF);
            break;
    }
}

void led_task_next_period(void){
    switch (half_period_ms){
        case 125:
        led_task_set_period_ms(500);
        break;
        case 250:
        led_task_set_period_ms(1000);
        break;
        case 500:
        led_task_set_period_ms(250);
        break;
        default:
        led_task_set_period_ms(250);
        break;
    }
}

void led_task_blink_state(void){
    led_task_set_state(LED_STATE_BLINK);
}

