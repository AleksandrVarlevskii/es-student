#include "platform.h"
#include "pico/stdlib.h"



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
#define BUTTON_PIN 15

static void platform_button_init(void){
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // включаем подтяжку к BUTTON_PIN, теперь там будет true в случае не нажатой кнопки и false в случае нажатой кнопки 
}
bool platform_button_read(void){
    return !gpio_get(BUTTON_PIN);
}








void platform_init(void){
    stdio_init_all();
    platform_led_init();
    platform_button_init();
    return;
}

