#pragma once

#include "pico/types.h"
#include <stdint.h>
#include <stdbool.h>

// Число секундомеров (номера 0..PROFILING_MAX_STOPWATCHES-1). Должно соотвествовать числу секундомеров объявленых в мейне
#ifndef PROFILING_MAX_STOPWATCHES
#define PROFILING_MAX_STOPWATCHES 5
#endif

// Результат замера одного секундомера.
//
// ВНИМАНИЕ: суммы замеров одного секундомера хранятся в 32 битах.
// Это дешевле, и сумма переполнится только через 71 минуту
// непрерывных замеров (2^32 мкс ≈ 71.6 мин).
typedef struct {
    const char *name;
    uint32_t    number_measurements; // число замеров
    uint64_t    mean;                // среднее арифметическое, мкс
    uint64_t    max;                 // максимум, мкс
    uint32_t    sum_us;
} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char *name); //функция инициилизации 

void profiling_start(uint32_t id); // начинаем профилирование
void profiling_stop(uint32_t id); // заканчиваем профилирование

// false — секундомер с таким номером не заведён
bool profiling_get(uint32_t id, profiling_result_t *result); // получаем результаты профизирования

void profiling_reset(void); // обнуляем все таймеры 