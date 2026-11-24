
#include "pi-task/pi-task.h"
#include "logging/log.h"
#include "systime/systime.h"


#define PI_TERMS_PER_CALL 90 // количество членов ряда, которые вычисляются за один вызов обработчика задачи. 
//Цена члена ряда, около 585 тактов. При 125 МГц это 4,7 мкс. Обработчик задачи должен уложиться в 0,5 мс, а это около ста членов.

// Состояние расчёта: должно пережить возврат из обработчика
static uint32_t pi_k;
static uint32_t pi_n;
static double pi_sum;
static double pi_sign;
static double pi_result;
static uint64_t pi_start_time_us;
static uint64_t pi_end_time_us;

static pi_task_state_t pi_state;
void pi_task_init(void)
{
    pi_state = PI_TASK_STATE_IDLE;
    pi_result = 0.0;
    
}

bool pi_task_start(uint32_t n)
{
    if (pi_state == PI_TASK_STATE_RUNNING) {
        return false; // уже запущено
    }
    
    pi_n = n;
    pi_k = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_state = PI_TASK_STATE_RUNNING;
    pi_start_time_us = systime_us();
    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_TASK_STATE_RUNNING) {
        return;
    }

    // Очередная порция членов ряда, но не дальше последнего
    int end = pi_k + PI_TERMS_PER_CALL;
    if (end > pi_n) {
        end = pi_n;
    }
    for (; pi_k < end; pi_k++) {
        pi_sum += pi_sign / (2.0 * pi_k + 1.0);
        pi_sign = -pi_sign;
    }

    if (pi_k == pi_n) {
        pi_result = 4.0 * pi_sum;
        pi_state = PI_TASK_STATE_READY;
        pi_end_time_us = systime_us();
    }
}

// Готов ли результат: если да, записать его по указателю
bool pi_task_get_result(double *result, uint64_t* time_us)
{
    if (pi_state != PI_TASK_STATE_READY) {
        return false;
    }
    *result = pi_result;
    if (time_us) { // проверяем что указатели не NULL, чтобы не было ошибки сегментации
        *time_us = pi_end_time_us - pi_start_time_us;
    } else {
        LOG_ERR("pi-task.c: pi_task_get_result: time_us is NULL; time was not written");
    }

    return true;
}

pi_task_state_t pi_task_get_state(void)
{
    return pi_state;
}