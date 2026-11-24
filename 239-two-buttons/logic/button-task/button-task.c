#include "button-task.h"

#include "systime/systime.h"
#include "logging/log.h"


#include <stddef.h> // для NULL


static const uint64_t BUTTON_DEBOUNCE_TIME = 20000; //(20 мс) порог времени в мкс, для защиты от времени (сколько сигнал должен продержаться что бы он был засчитан как нажатие кнопки) 
static const uint64_t BUTTON_LONG_PRESSING = 1000000; // (1 секунда) порог времени в мкс какое время достаточно подождать, для засчета долгого нажатия. 


typedef enum {
    BUTTON_STATE_PRESSED, // нажата
    BUTTON_STATE_PRESSED_DEBOUNCE, // нажата? (ждем, что бы проверить не дребезг ли )
    BUTTON_STATE_RELEASED, // отжата
    BUTTON_STATE_RELEASED_DEBOUNCE, //отжата? (ждем, что бы проверить не дребезг ли )
} button_state_t;

typedef struct {
    button_state_t state;
    platform_button_t platform_button;
    button_task_callback_t on_fast_press_callback;
    button_task_callback_t on_long_press_callback;
    uint64_t last_toggle_us;
    uint64_t press_start_us;
    uint32_t click_counter;
} button_t;


static button_t buttons[BUTTON_COUNT];

static button_t* get_button_t(platform_button_t platform_button){
    for (int i=0; i< BUTTON_COUNT; i++){
        if (buttons[i].platform_button == platform_button) {return &buttons[i];}
    }
    LOG_ERR("button_task: get_button_t : The corresponding platform_button_t value passed to the function was not found among the buttons. The first button was passed");
    return &buttons[0];
}

const char* get_button_statename(platform_button_t platform_button)
{
    button_t* button = get_button_t(platform_button);
    switch (button->state)
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



bool button_task_is_pressed(platform_button_t platform_button){
    return get_button_t(platform_button)->state == BUTTON_STATE_PRESSED;
}
uint32_t button_task_get_press_count(platform_button_t platform_button){
    return get_button_t(platform_button)->click_counter;
}



void button_task_init(button_task_callback_t* arr_on_press){
    
    for (int j=0; j<BUTTON_COUNT; j++){
        button_t button = {
            .state = BUTTON_STATE_RELEASED,
            .platform_button = (platform_button_t)j,
            .on_fast_press_callback = (arr_on_press != NULL) ? arr_on_press[2 * j]     : NULL,
            .on_long_press_callback = (arr_on_press != NULL) ? arr_on_press[2 * j + 1] : NULL,
            .click_counter = 0,
            .last_toggle_us = systime_us(),
            .press_start_us = 0,
            
        }; 
        buttons[j] = button;
    }
    
    // for (int j=0; j< BUTTON_COUNT; j++){
    //         if(platform_button_read(buttons[j].platform_button)){
    //     buttons[j].state= BUTTON_STATE_PRESSED_DEBOUNCE;
    // }else {
    //     buttons[j].state = BUTTON_STATE_RELEASED_DEBOUNCE;
    // }
    // }


}



void button_task_handle(void)
{
    uint64_t now_us = systime_us();
    for (int j=0; j<BUTTON_COUNT; j++){
        button_t* button = &buttons[j];
        bool pressed = platform_button_read(button->platform_button);
        

        switch (button->state)
        {
            case BUTTON_STATE_PRESSED:
                if (pressed == false){ // кнопка отжата
                    button->state = BUTTON_STATE_RELEASED_DEBOUNCE;
                    button->last_toggle_us = now_us; 
                }
                
                break;
            case BUTTON_STATE_RELEASED:
                if (pressed == true){ // кнопка нажата
                    button->state = BUTTON_STATE_PRESSED_DEBOUNCE;
                    button->last_toggle_us = now_us;
                    button->press_start_us = now_us;  
                }
                break;
            case BUTTON_STATE_RELEASED_DEBOUNCE:
                if (pressed == false && now_us - button->last_toggle_us >= BUTTON_DEBOUNCE_TIME){ // кнопка отжата и прошло время дребезга
                    button->state = BUTTON_STATE_RELEASED;
                    if (button->press_start_us != 0 && now_us - button->press_start_us >= BUTTON_LONG_PRESSING && button->on_long_press_callback != NULL ){
                        button->on_long_press_callback();
                    }else{
                        if (button->on_fast_press_callback != NULL){button->on_fast_press_callback();}
                    }
                    button->click_counter++;
                }else if (pressed == true){
                    button->state = BUTTON_STATE_PRESSED_DEBOUNCE;
                    button->last_toggle_us = now_us; 
                    button->press_start_us = now_us; 
                }
                break;
            case BUTTON_STATE_PRESSED_DEBOUNCE:
                if (pressed == true && now_us - button->last_toggle_us >= BUTTON_DEBOUNCE_TIME){ // кнопка нажата и прошло время дребезга
                    button->state = BUTTON_STATE_PRESSED;
                }else if(pressed == false){
                    button->state = BUTTON_STATE_RELEASED_DEBOUNCE;
                    button->last_toggle_us = now_us; 
                }
                break;
        }
    }
}