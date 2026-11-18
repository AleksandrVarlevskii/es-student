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
#define EMA_ONE_MINUS_Q15   (32768 - EMA_ALPHA_Q15)

static int32_t ema_state_ohm = 0;
static bool    ema_init      = false;

static inline int32_t ema_q15(int32_t x_ohm) {
    if (!ema_init) {
        ema_state_ohm = x_ohm;
        ema_init = true;
        return ema_state_ohm;
    }
    int64_t acc = (int64_t)EMA_ALPHA_Q15     * x_ohm
                + (int64_t)EMA_ONE_MINUS_Q15 * ema_state_ohm;
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
    pwm_set_gpio_level(pin, (SERVO_MIN_US + SERVO_MAX_US) / 2); // 90 градусов
}

static void servo_set_angle(float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    float pulse_us = SERVO_MIN_US
                   + (angle / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US);
    pwm_set_gpio_level(SERVO_PIN, (uint16_t)(pulse_us + 0.5f));
}

// ---------- ДИНАМИЧЕСКОЕ СРАВНЕНИЕ ----------
#define HISTORY_SIZE 2000        // Сколько последних отсчетов помнить (~2 секунды при 1000 Гц)
#define THRESHOLD_DELTA 1500    // Порог изменения (в Омах). Если R изменилось на 20 кОм — реагируем

static int32_t r_history[HISTORY_SIZE];
static int history_index = 0;
static bool history_filled = false;

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    servo_init(SERVO_PIN);

    // Заполняем историю стартовым значением, чтобы не было прыжка в начале
    for (int i = 0; i < HISTORY_SIZE; i++) {
        uint16_t raw = adc_read();
        r_history[i] = adc_to_r_ohm(raw);
    }
    
    int32_t running_sum = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) running_sum += r_history[i];

    uint32_t n = 0;
    float last_angle = 90.0f; // Начинаем с центра

    while (1) {
        uint16_t raw = adc_read();
        int32_t R_raw = adc_to_r_ohm(raw);
        int32_t R_f   = ema_q15(R_raw); // Текущее отфильтрованное значение

        // 1. Вычисляем среднее значение за последнее время (N измерений)
        int32_t R_avg = running_sum / HISTORY_SIZE;

        // 2. Смотрим, насколько текущее значение отличается от среднего
        int32_t delta = R_f - R_avg;

        // 3. Управляем сервой
        float target_angle = 90.0f; // По умолчанию стоим в центре (90°)

        if (delta > THRESHOLD_DELTA) {
            // Сопротивление РЕЗКО ВЫРОСЛО (стало больше среднего)
            target_angle = 170.0f; // Уходим в один край
        } 
        else if (delta < -THRESHOLD_DELTA) {
            // Сопротивление РЕЗКО УПАЛО (стало меньше среднего)
            target_angle = 10.0f;  // Уходим в другой край
        }

        // Двигаем серву только если угол изменился больше чем на 0.5 градуса
        if (fabs(target_angle - last_angle) > 0.5f) {
            servo_set_angle(target_angle);
            last_angle = target_angle;
        }

        // 4. Обновляем историю (выкидываем старый элемент, добавляем новый)
        running_sum -= r_history[history_index];
        r_history[history_index] = R_f;
        running_sum += R_f;
        
        history_index++;
        if (history_index >= HISTORY_SIZE) {
            history_index = 0;
        }

        // Печать раз в секунду
        if ((n % 1000) == 0) {
            printf("R_curr: %ld | R_avg: %ld | Delta: %ld | Angle: %.1f\n", 
                   (long)R_f, (long)R_avg, (long)delta, target_angle);
        }
        n++;

        gpio_put(LED_PIN, (n / 1000) & 1);
        sleep_ms(1);
    }
}