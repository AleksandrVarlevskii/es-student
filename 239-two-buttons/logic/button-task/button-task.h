#pragma once
// button_task.h задача кнопки, обнаружить нажатие кнопки, отличив его от дребизга и в случае нажатия переключить режим работы светодиода в задаче led-task
#include <stdbool.h>
#include <stdint.h>

#include "platform/platform.h"
// функция, которую задача вызывает на каждое нажатие
typedef void (*button_task_callback_t)(void);

void button_task_handle(void); // обработчик задачи кнопка



void button_task_init(button_task_callback_t* arr_on_press); // получает указатель на массив размера bUTTON_COUNT, где лежат указатели на функуии что выполняют кнопки. первая функция это первая кнопка.


bool button_task_is_pressed(platform_button_t button); // нигде не используется, нужно только для по требованию задания
uint32_t button_task_get_press_count(platform_button_t button); // возращает количество нажатий кнопки
const char* get_button_statename(platform_button_t button); // возращает название состояния кнопки на английском языке: pressed, released, pressed (debounce), released (debounce)