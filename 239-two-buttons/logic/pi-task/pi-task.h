// pi-task.h задача, что считает число Пи.
#pragma once

#include <stdint.h>
#include <stdbool.h>



typedef enum {
    PI_TASK_STATE_IDLE,
    PI_TASK_STATE_RUNNING,
    PI_TASK_STATE_READY,
} pi_task_state_t;

void pi_task_handle(void); // обработчик задачи
void pi_task_init(void); // инициилизация задачи
bool pi_task_get_result(double *result, uint64_t *time_us); // готов ли результат, если да возращает true и записавает его по указателю, иначе false
bool pi_task_start(uint32_t n); // запуск задачи на вычисление числа Пи с количеством членов ряда n . Если задача уже запущена, то возращает false, иначе true.
pi_task_state_t pi_task_get_state(void); // возращает состояние задачи