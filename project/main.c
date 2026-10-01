#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

const uint ADC_PIN = 26;        // ADC0 физический номер - 31
const uint LED_PIN = 25;

int main() {
    stdio_init_all();           // инициализация USB CDC (COM-порт)

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_PIN);     // настроить GPIO26 как аналоговый вход
    adc_select_input(0);        // ADC0

    while (1) {
        uint16_t raw = adc_read();                    // 0..4095
        float voltage = raw * 3.3f / 4095.0f;         // перевод в вольты

        printf("ADC: %4u  Voltage: %.3f V\n", raw, voltage);
        sleep_ms(1);
    }
}