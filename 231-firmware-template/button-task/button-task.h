#pragma once
// button_task.h задача кнопки, обнаружить нажатие кнопки, отличив его от дребизга и в случае нажатия переключить режим работы светодиода в задаче led-task

#include "led-task.h"
typedef enum {
    BUTTON_STATE_PRESSED, // нажата
    BUTTON_STATE_PRESSED_DEBOUNCE, // нажата? (ждем, что бы проверить не дребезг ли )
    BUTTON_STATE_RELEASED, // отжата
    BUTTON_STATE_RELEASED_DEBOUNCE, //отжата? (ждем, что бы проверить не дребезг ли )
} button_state_t;

void button_task_handle(void); // обработчик задачи кнопка

void button_task_init(void); // инициилизация задачи кнопка


bool button_task_is_pressed(void); // нигде не используется, нужно только для по требованию задания
uint32_t button_task_get_press_count(void); // возращает количество нажатий кнопки
const char* get_button_statename(void); // возращает название состояния кнопки на английском языке: pressed, released, pressed (debounce), released (debounce)