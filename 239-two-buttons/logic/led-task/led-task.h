// led-task.h — задача светодиода: выключен, горит или мигает
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


typedef enum {
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_BLINK,
} led_state_t;

void led_task_init(void); // функция иницилизации (начальное состояние мигать)
void led_task_handle(void); // функция обработчик 

void led_task_set_state(led_state_t state); // устанавливает новое состояние led: включен, выключен, мигает
led_state_t led_task_get_state(void); // возрощает состояние

// false — период не принят
bool led_task_set_period_ms(uint32_t period_ms); // устанавливает новый период для мигания, если он равен 0 то возращает false
uint32_t led_task_get_period_ms(void); // возрощает переиод мигания led

void led_task_next_state(void); // переключает состояние led на следующее: выключен, включен, мигает
void led_task_next_period(void); // переключает значения периодна на следующие: 250-500-1000 мл и так по кругу. Если установлен другой период, то устанавливает 250.
void led_task_blink_state(void); // устанавливает стостояние мигания 