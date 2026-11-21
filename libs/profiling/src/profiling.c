#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "pico/time.h"

#include "logging/log.h"
#include "profiling/profiling.h"
#include "systime/systime.h"


typedef struct {
    const char *name;       // NULL — секундомер не заведён
    uint32_t    count;      // число замеров
    uint32_t    sum_us;     // сумма замеров (32 бита, см. комментарий в profiling.h)
    uint32_t    start_us;   // момент старта
    uint32_t    max_us;     // максимум
    bool        running;    // защита от повторного старта
} stopwatch_t;

static stopwatch_t s_stopwatches[PROFILING_MAX_STOPWATCHES];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_MAX_STOPWATCHES){
        LOG_ERR("profiling_stopwatch_init: id %u is out of range (max %u). "
                "Increase PROFILING_MAX_STOPWATCHES in profiling.h if more stopwatches are needed.",
                (unsigned)id, (unsigned)PROFILING_MAX_STOPWATCHES);
        return;
    }
    if (name == NULL){
        LOG_ERR("profiling_stopwatch_init: name is NULL for id %u. "
                "A stopwatch must have a non-NULL name to be registered.",
                (unsigned)id);
        return;
    }
    stopwatch_t *sw = &s_stopwatches[id];
    sw->name     = name;
    sw->count    = 0;
    sw->sum_us   = 0;
    sw->start_us = 0;
    sw->max_us   = 0;
    sw->running  = false;
}


void profiling_start(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES){
        LOG_ERR("profiling_start: id %u is out of range (max %u). "
                "Increase PROFILING_MAX_STOPWATCHES in profiling.h if more stopwatches are needed.",
                (unsigned)id, (unsigned)PROFILING_MAX_STOPWATCHES);
        return;
    }
    stopwatch_t *sw = &s_stopwatches[id];
    if (sw->name == NULL){
        LOG_ERR("profiling_start: stopwatch %u is not registered. "
                "Call profiling_stopwatch_init() for it before starting.",
                (unsigned)id);
        return;
    }
    if (sw->running){
        LOG_ERR("profiling_start: stopwatch %u ('%s') is already running. "
                "Call profiling_stop() before starting it again.",
                (unsigned)id, sw->name);
        return;
    }
    sw->start_us = (uint32_t)systime_us();
    sw->running  = true;
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES){
        LOG_ERR("profiling_stop: id %u is out of range (max %u). "
                "Increase PROFILING_MAX_STOPWATCHES in profiling.h if more stopwatches are needed.",
                (unsigned)id, (unsigned)PROFILING_MAX_STOPWATCHES);
        return;
    }
    stopwatch_t *sw = &s_stopwatches[id];
    if (sw->name == NULL){
        LOG_ERR("profiling_stop: stopwatch %u is not registered. "
                "Call profiling_stopwatch_init() for it before using it.",
                (unsigned)id);
        return;
    }
    if (!sw->running){
        LOG_ERR("profiling_stop: stopwatch %u ('%s') is not running. "
                "Call profiling_start() before profiling_stop().",
                (unsigned)id, sw->name);
        return;
    }

    uint32_t now = (uint32_t)systime_us();
    uint32_t dt  = now - sw->start_us; // беззнаковая разность корректна и при переполнении time_us_32

    sw->running = false;
    sw->count++;
    sw->sum_us += dt;                  // 32-битная сумма, переполнение ~71 мин
    if (dt > sw->max_us) sw->max_us = dt;
}



bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= PROFILING_MAX_STOPWATCHES){
        LOG_ERR("profiling_get: id %u is out of range (max %u). "
                "Increase PROFILING_MAX_STOPWATCHES in profiling.h if more stopwatches are needed.",
                (unsigned)id, (unsigned)PROFILING_MAX_STOPWATCHES);
        return false;
    }
    if (result == NULL){
        LOG_ERR("profiling_get: result pointer is NULL for id %u. "
                "Pass a valid profiling_result_t* to receive the data.",
                (unsigned)id);
        return false;
    }
    const stopwatch_t *sw = &s_stopwatches[id];
    if (sw->name == NULL){
        LOG_ERR("profiling_get: stopwatch %u is not registered. "
                "Call profiling_stopwatch_init() for it before reading.",
                (unsigned)id);
        return false;
    }

    result->name                = sw->name;
    result->number_measurements = sw->count;
    result->mean                = (sw->count > 0)
                                    ? (uint64_t)sw->sum_us / sw->count
                                    : 0;
    result->max                 = sw->max_us;
    result->sum_us                = sw->sum_us;
    return true;
}

void profiling_reset(void)
{
    // Ошибок быть не может: проходим по всему массиву,
    // у незаведённых секундомеров name == NULL и обнуление полей безвредно.
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++)
    {
        stopwatch_t *sw = &s_stopwatches[i];
        sw->count    = 0;
        sw->sum_us   = 0;
        sw->max_us   = 0;
        // name сохраняется — секундомер остаётся заведённым
        // running не сбрасываем иначе будет ошибка с profiling_stop: stopwatch %u ('%s') is not running. 
        /* start_us не сбрасывем иначе в profiling_stop будет now-0, что сломает все статистику, 
        при этом сбрасывать и не надо т.к. беззнаковая разность корректна и при переполнении time_us_32.
        Проблема может возникнуть только если переполнение случится более одного раза, но это означало бы что одна итерация цикла заняла больше 70 минут.*/
    }
}