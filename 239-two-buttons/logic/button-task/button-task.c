#include "button-task.h"

#include "systime/systime.h"
#include "logging/log.h"


#include <stddef.h> // для NULL


static const uint64_t BUTTON_DEBOUNCE_TIME = 20000; // порог времени, для защиты от времени (сколько сигнал должен продержаться что бы он был засчитан как нажатие кнопки) 
static uint64_t last_toggle_us[BUTTON_COUNT] = {0, 0};
static uint32_t click_counter[BUTTON_COUNT] = {0, 0}; // счетчик нажатий
 
static button_task_callback_t* arr_on_press_callback = NULL;

typedef enum {
    BUTTON_STATE_PRESSED, // нажата
    BUTTON_STATE_PRESSED_DEBOUNCE, // нажата? (ждем, что бы проверить не дребезг ли )
    BUTTON_STATE_RELEASED, // отжата
    BUTTON_STATE_RELEASED_DEBOUNCE, //отжата? (ждем, что бы проверить не дребезг ли )
} button_state_t;

button_state_t button_state[BUTTON_COUNT]; 


const char* get_button_statename(platform_button_t button)
{
    switch (button_state[button])
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



bool button_task_is_pressed(platform_button_t button){
    return button_state[button] == BUTTON_STATE_PRESSED;
}
uint32_t button_task_get_press_count(platform_button_t button){
    return click_counter[button];
}



void button_task_init(button_task_callback_t* arr_on_press){
    arr_on_press_callback = arr_on_press;
    if (arr_on_press == NULL){ LOG_ERR("button_task.c : button_task_init: function button_task_init was passed NULL instead of a pointer to an array of functions"); }
    for (int button_i=0; button_i< BUTTON_COUNT; button_i++){
        if (arr_on_press[button_i]== NULL){
            LOG_ERR("button_task.c : button_task_init : NULL was passed to the %i button instead of a function pointer.", button_i);
        }
    }
    for (int button_i =0; button_i< BUTTON_COUNT; button_i++){
            if(platform_button_read(button_i)){
        button_state[button_i] = BUTTON_STATE_PRESSED_DEBOUNCE;
    }else {
        button_state[button_i] = BUTTON_STATE_RELEASED_DEBOUNCE;
    }
    last_toggle_us[button_i] = systime_us(); // сбрасываем счетчик времени.
    }


}



void button_task_handle(void)
{
    for (int button_i=0; button_i<BUTTON_COUNT; button_i++){
        bool pressed = platform_button_read(button_i);
        uint64_t now_us = systime_us();
        
        switch (button_state[button_i])
        {
            case BUTTON_STATE_PRESSED:
                if (pressed == false){ // кнопка отжата
                    button_state[button_i] = BUTTON_STATE_RELEASED_DEBOUNCE;
                    last_toggle_us[button_i] = now_us; 
                }
                break;
            case BUTTON_STATE_RELEASED:
                if (pressed == true){ // кнопка нажата
                    button_state[button_i] = BUTTON_STATE_PRESSED_DEBOUNCE;
                    last_toggle_us[button_i] = now_us;
                }
                break;
            case BUTTON_STATE_RELEASED_DEBOUNCE:
                if (pressed == false && now_us - last_toggle_us[button_i] >= BUTTON_DEBOUNCE_TIME){ // кнопка отжата и прошло время дребезга
                    button_state[button_i] = BUTTON_STATE_RELEASED;

                }
                if (pressed == true){
                    button_state[button_i] = BUTTON_STATE_PRESSED_DEBOUNCE;
                    last_toggle_us[button_i] = now_us; 
                }
                break;
            case BUTTON_STATE_PRESSED_DEBOUNCE:
                if (pressed == true && now_us - last_toggle_us[button_i] >= BUTTON_DEBOUNCE_TIME){ // кнопка нажата и прошло время дребезга
                    button_state[button_i] = BUTTON_STATE_PRESSED;
                    if (arr_on_press_callback[button_i] != NULL){
                    arr_on_press_callback[button_i]();
                    }else{
                        LOG_DBG("button_task.c : button_task_handle : button %i tries to call the NULL function, but nothing was called instead.", button_i);
                    }
                    click_counter[button_i]++;
                }
                if (pressed == false){
                    button_state[button_i] = BUTTON_STATE_RELEASED_DEBOUNCE;
                    last_toggle_us[button_i] = now_us; 
                }
                break;
        }
    }
}