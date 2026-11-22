#include "button-task.h"

#include "systime/systime.h"
#include "platform/platform.h"

#include <stddef.h> // для NULL


static const uint64_t BUTTON_DEBOUNCE_TIME = 20000; // порог времени, для защиты от времени (сколько сигнал должен продержаться что бы он был засчитан как нажатие кнопки) 
static uint64_t last_toggle_us = 0;
static uint32_t click_counter = 0; // счетчик нажатий
 
static button_task_callback_t on_press_callback = NULL;

typedef enum {
    BUTTON_STATE_PRESSED, // нажата
    BUTTON_STATE_PRESSED_DEBOUNCE, // нажата? (ждем, что бы проверить не дребезг ли )
    BUTTON_STATE_RELEASED, // отжата
    BUTTON_STATE_RELEASED_DEBOUNCE, //отжата? (ждем, что бы проверить не дребезг ли )
} button_state_t;

button_state_t button_state; 


const char* get_button_statename(void)
{
    switch (button_state)
    {
        case BUTTON_STATE_PRESSED:
            return "pressed";
        case BUTTON_STATE_PRESSED_DEBOUNCE:
            return "pressed (debounce)";
        case BUTTON_STATE_RELEASED:
            return "released";
        case BUTTON_STATE_RELEASED_DEBOUNCE:
            return "released (debounce)";
        default:
            return "unknown";
    }
}



bool button_task_is_pressed(){
    return button_state == BUTTON_STATE_PRESSED;
}
uint32_t button_task_get_press_count(void){
    return click_counter;
}



void button_task_init(button_task_callback_t on_press){
    on_press_callback = on_press;
    if(platform_button_read()){
        button_state = BUTTON_STATE_PRESSED_DEBOUNCE;
    }else {
        button_state = BUTTON_STATE_RELEASED_DEBOUNCE;
    }
    last_toggle_us = systime_us(); // сбрасываем счетчик времени.
}



void button_task_handle(void)
{
    bool pressed = platform_button_read();
    uint64_t now_us = systime_us();

    switch (button_state)
    {
        case BUTTON_STATE_PRESSED:
            if (pressed == false){ // кнопка отжата
                button_state = BUTTON_STATE_RELEASED_DEBOUNCE;
                last_toggle_us = now_us; 
            }
            break;
        case BUTTON_STATE_RELEASED:
            if (pressed == true){ // кнопка нажата
                button_state = BUTTON_STATE_PRESSED_DEBOUNCE;
                last_toggle_us = now_us;
            }
            break;
        case BUTTON_STATE_RELEASED_DEBOUNCE:
            if (pressed == false && now_us - last_toggle_us >= BUTTON_DEBOUNCE_TIME){ // кнопка отжата и прошло время дребезга
                button_state = BUTTON_STATE_RELEASED;

            }
            if (pressed == true){
                button_state = BUTTON_STATE_PRESSED_DEBOUNCE;
                last_toggle_us = now_us; 
            }
            break;
        case BUTTON_STATE_PRESSED_DEBOUNCE:
            if (pressed == true && now_us - last_toggle_us >= BUTTON_DEBOUNCE_TIME){ // кнопка нажата и прошло время дребезга
                button_state = BUTTON_STATE_PRESSED;
                on_press_callback();
                click_counter++;
            }
            if (pressed == false){
                button_state = BUTTON_STATE_RELEASED_DEBOUNCE;
                last_toggle_us = now_us; 
            }
            break;
    }
}