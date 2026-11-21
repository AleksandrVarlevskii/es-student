#pragma once


#include <stdint.h>
#include <stdbool.h>

#include "pico/types.h"

// Число секундомеров (номера 0..PROFILING_MAX_STOPWATCHES-1). Должно соотвествовать числу секундомеров объявленых в мейне
#ifndef PROFILING_MAX_STOPWATCHES
#define PROFILING_MAX_STOPWATCHES 5
#endif

// по умолчанию профилирование включено
#ifndef PROFILING_ENABLED
#define PROFILING_ENABLED 1
#endif

#if PROFILING_ENABLED
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
#else
// профилирование выключено: вызовы ничего не делают

#define profiling_start(id) ((void)0)
#define profiling_stop(id) ((void)0)
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



// false — секундомер с таким номером не заведён
bool profiling_get(uint32_t id, profiling_result_t *result); // получаем результаты профизирования

void profiling_reset(void); // обнуляем все таймеры 