#include "platform.h"
#include "pico/stdlib.h"
#include "logging/log.h"

#define BUTTON_0_PIN 15
#define BUTTON_1_PIN 14

//LED
#define LED_PIN PICO_DEFAULT_LED_PIN

static void platform_led_init(void){
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void platform_led_set(bool on){
    gpio_put(LED_PIN, on);
}

bool platform_led_get(void){
    return gpio_get(LED_PIN);
}

// BUTTON
static int get_button_pin(platform_button_t button){
    switch (button) {
        case BUTTON_0:
            return BUTTON_0_PIN;
            break;
        case BUTTON_1:
            return BUTTON_1_PIN;
            break;
        default:
            LOG_ERR("platform.c: get_button_pin: received an unknown button, the pin of the button was set to 28.");
            return 28;
    }
}

static void platform_button_init(void){
    for (int button_i=BUTTON_0; button_i < BUTTON_COUNT; button_i++){
        int button_i_pin = get_button_pin((platform_button_t)button_i);
        gpio_init(button_i_pin);
        gpio_set_dir(button_i_pin, GPIO_IN);
        gpio_pull_up(button_i_pin); // включаем подтяжку к BUTTON_PIN, теперь там будет true в случае не нажатой кнопки и false в случае нажатой кнопки 
    }
    
}
bool platform_button_read(platform_button_t button){
    return !gpio_get(get_button_pin(button));
}








void platform_init(void){
    stdio_init_all();
    platform_led_init();
    platform_button_init();
    return;
}

