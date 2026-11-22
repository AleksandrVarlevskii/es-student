#pragma once
// button_task.h задача кнопки, обнаружить нажатие кнопки, отличив его от дребизга и в случае нажатия переключить режим работы светодиода в задаче led-task
#include <stdbool.h>
#include <stdint.h>

// функция, которую задача вызывает на каждое нажатие
typedef void (*button_task_callback_t)(void);

void button_task_handle(void); // обработчик задачи кнопка



void button_task_init(button_task_callback_t on_press);


bool button_task_is_pressed(void); // нигде не используется, нужно только для по требованию задания
uint32_t button_task_get_press_count(void); // возращает количество нажатий кнопки
const char* get_button_statename(void); // возращает название состояния кнопки на английском языке: pressed, released, pressed (debounce), released (debounce)