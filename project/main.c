#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

const uint ADC_PIN = 26;
const uint LED_PIN = 25;

// ---------- параметры делителя ----------
#define G          3.2f
#define V_REF      3.3f
#define R0         5000000.0f   // 5 МОм

// ---------- EMA q15 ----------
// alpha = 0.03  ->  0.03 * 32768 = 983
#define EMA_ALPHA_Q15       983
#define EMA_ONE_MINUS_Q15   (32768 - EMA_ALPHA_Q15)

static int32_t ema_state_ohm = 0;
static bool    ema_init      = false;

static inline int32_t ema_q15(int32_t x_ohm) {
    if (!ema_init) {
        ema_state_ohm = x_ohm;
        ema_init = true;
        return ema_state_ohm;
    }
    // int64, чтобы избежать переполнения int32:
    //   EMA_ONE_MINUS_Q15 * ema_state_ohm ~ 31785 * 4e5 ~ 1.3e10
    int64_t acc = (int64_t)EMA_ALPHA_Q15     * x_ohm
                + (int64_t)EMA_ONE_MINUS_Q15 * ema_state_ohm;
    ema_state_ohm = (int32_t)(acc >> 15);
    return ema_state_ohm;
}

// ---------- ADC -> R в Омах (int32) ----------
static inline int32_t adc_to_r_ohm(uint16_t raw) {
    // voltage = raw * 3.3 / 4095
    // R = v * R0 / (G*V_REF - v)
    float v = raw * (V_REF / 4095.0f);
    float denom = G * V_REF - v;
    if (denom <= 1e-3f) return 0;
    float R = v * R0 / denom;
    if (R < 0) R = 0;
    if (R > 200000000.0f) R = 200000000.0f;
    return (int32_t)(R + 0.5f);
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    // Прогрев EMA первым отсчётом
    {
        uint16_t raw = adc_read();
        (void)ema_q15(adc_to_r_ohm(raw));
    }

    uint32_t n = 0;
    while (1) {
        uint16_t raw = adc_read();
        int32_t R_raw = adc_to_r_ohm(raw);
        int32_t R_f   = ema_q15(R_raw);

        // Печатаем каждый 10-й отсчёт (~100 Гц на USB CDC)
        if ((n % 10) == 0) {
            printf("R: %ld %ld\n", (long)R_raw, (long)R_f);
        }
        n++;

        // мигаем светодиодом раз в секунду
        gpio_put(LED_PIN, (n / 1000) & 1);

        sleep_ms(1);
    }
}