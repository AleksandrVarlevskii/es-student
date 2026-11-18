#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

const uint ADC_PIN   = 26;
const uint LED_PIN   = 25;
const uint SERVO_PIN = 15;

// ---------- параметры делителя ----------
#define G          3.17f
#define V_REF      3.3f
#define R0         10400000.0f

// ---------- EMA q15 ----------
#define EMA_ALPHA_Q15       983
static int32_t ema_state_ohm = 0;
static bool    ema_init      = false;

static inline int32_t ema_q15(int32_t x_ohm) {
    if (!ema_init) { ema_state_ohm = x_ohm; ema_init = true; return ema_state_ohm; }
    int64_t acc = (int64_t)EMA_ALPHA_Q15 * x_ohm + (int64_t)(32768 - EMA_ALPHA_Q15) * ema_state_ohm;
    ema_state_ohm = (int32_t)(acc >> 15);
    return ema_state_ohm;
}

// ---------- ADC -> R в Омах ----------
static inline int32_t adc_to_r_ohm(uint16_t raw) {
    float v = raw * (V_REF / 4095.0f);
    float denom = G * V_REF - v;
    if (denom <= 1e-3f) return 0;
    float R = v * R0 / denom;
    if (R < 0) R = 0;
    if (R > 200000000.0f) R = 200000000.0f;
    return (int32_t)(R + 0.5f);
}

// ---------- СЕРВО ----------
#define SERVO_PERIOD_US   20000
#define SERVO_MIN_US      544
#define SERVO_MAX_US      2400
static uint servo_slice;

static void servo_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    servo_slice = pwm_gpio_to_slice_num(pin);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 125.0f);
    pwm_config_set_wrap(&cfg, SERVO_PERIOD_US - 1);
    pwm_init(servo_slice, &cfg, true);
    pwm_set_gpio_level(pin, (SERVO_MIN_US + SERVO_MAX_US) / 2); 
}

static void servo_set_angle(float angle) {
    if (angle < 10.0f) angle = 10.0f;
    if (angle > 170.0f) angle = 170.0f;
    float pulse_us = SERVO_MIN_US + (angle / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US);
    pwm_set_gpio_level(SERVO_PIN, (uint16_t)(pulse_us + 0.5f));
}

// ---------- МЕДИАННЫЙ ФИЛЬТР (от шума) ----------
#define MED_SIZE 5
static int32_t med_buf[MED_SIZE] = {0};
static int med_idx = 0;
static int32_t filter_median(int32_t new_val) {
    med_buf[med_idx] = new_val;
    med_idx = (med_idx + 1) % MED_SIZE;
    int32_t temp[MED_SIZE];
    for(int i=0; i<MED_SIZE; i++) temp[i] = med_buf[i];
    for (int i = 1; i < MED_SIZE; i++) {
        int32_t key = temp[i]; int j = i - 1;
        while (j >= 0 && temp[j] > key) { temp[j + 1] = temp[j]; j--; }
        temp[j + 1] = key;
    }
    return temp[MED_SIZE / 2];
}

// ---------- ГЛАВНАЯ ЛОГИКА (АБСОЛЮТНАЯ) ----------
// ВАЖНО: Подставьте сюда реальные сопротивления, которые показывает ваш потенциометр!
// Например, если минимум 100 000 (100 кОм), а максимум 1 500 000 (1.5 МОм):
#define R_MIN 100000.0f
#define R_MAX 1500000.0f

int main() {
    stdio_init_all();
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    adc_init(); adc_gpio_init(ADC_PIN); adc_select_input(0);
    servo_init(SERVO_PIN);

    // Прогрев EMA
    for(int i=0; i<100; i++) { ema_q15(adc_to_r_ohm(adc_read())); sleep_ms(2); }

    uint32_t n = 0;

    while (1) {
        int32_t R_raw = adc_to_r_ohm(adc_read());
        int32_t R_med = filter_median(R_raw);
        int32_t R_f   = ema_q15(R_med);

        // 1. Ограничиваем сопротивление рамками (защита от вылета в бесконечность)
        float r_clamped = (float)R_f;
        if (r_clamped < R_MIN) r_clamped = R_MIN;
        if (r_clamped > R_MAX) r_clamped = R_MAX;

        // 2. Линейно переводим сопротивление в угол (от 10 до 170)
        float ratio = (r_clamped - R_MIN) / (R_MAX - R_MIN);
        float target_angle = 10.0f + (ratio * 160.0f);

        // 3. Отправляем на серву
        servo_set_angle(target_angle);

        // Печать в консоль
        if ((n % 100) == 0) {
            printf("R: %ld Ohm | Ratio: %.2f | Angle: %.1f\n", 
                   (long)R_f, ratio, target_angle);
        }
        n++;
        gpio_put(LED_PIN, (n / 1000) & 1);
        sleep_ms(2);
    }
}