#pragma once

#include <stdbool.h>



// запускает стандартный ввод-вывод и настраивает выводы
void platform_init(void);

void platform_led_set(bool on); // включает и выключает led
bool platform_led_get(void); // возрощает включен led или нет (true or false)

// нажата ли кнопка сейчас, без подавления дребезга
bool platform_button_read(void);