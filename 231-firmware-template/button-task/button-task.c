#include "button-task.h"


static const uint BUTTON_PIN = 15; // пин кнопки
static const uint64_t BUTTON_DEBOUNCE_TIME = 20000; // порог времени, для защиты от времени (сколько сигнал должен продержаться что бы он был засчитан как нажатие кнопки) 
static uint64_t last_toggle_us = 0;
static uint32_t click_counter = 0; // счетчик нажатий
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

void print_button_state(void)
{  
printf("button: %s, presses %u\r\n", get_button_statename(), (unsigned)click_counter);
}

bool button_task_is_pressed(){
    return button_state == BUTTON_STATE_PRESSED ;
}
uint32_t button_task_get_press_count(void){
    return click_counter;
}



void button_task_init(void){
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // включаем подтяжку к BUTTON_PIN, теперь там будет true в случае не нажатой кнопки и false в случае нажатой кнопки 
    if(gpio_get(BUTTON_PIN) == true){
        button_state = BUTTON_STATE_RELEASED_DEBOUNCE;
    }else {
        button_state = BUTTON_STATE_PRESSED_DEBOUNCE;
    }
    last_toggle_us = time_us_64(); // сбрасываем счетчик времени.
}



void button_task_handle(void)
{
    bool level = gpio_get(BUTTON_PIN);
    uint64_t now_us = time_us_64();

    switch (button_state)
    {
        case BUTTON_STATE_PRESSED:
            if (level == true){ // кнопка отжата
                button_state = BUTTON_STATE_RELEASED_DEBOUNCE;
                last_toggle_us = now_us; // сбрасываем счетчик времени.
            }
            break;
        case BUTTON_STATE_RELEASED:
            if (level == false){ // кнопка нажата
                button_state = BUTTON_STATE_PRESSED_DEBOUNCE;
                last_toggle_us = now_us; // сбрасываем счетчик времени.
            }
            break;
        case BUTTON_STATE_RELEASED_DEBOUNCE:
            if (level == true && now_us - last_toggle_us >= BUTTON_DEBOUNCE_TIME){ // кнопка отжата и прошло время дребезга
                button_state = BUTTON_STATE_RELEASED;

            }
            if (level == false){
                button_state = BUTTON_STATE_PRESSED_DEBOUNCE;
                last_toggle_us = now_us; // сбрасываем счетчик времени.
            }
            break;
        case BUTTON_STATE_PRESSED_DEBOUNCE:
            if (level == false && now_us - last_toggle_us >= BUTTON_DEBOUNCE_TIME){ // кнопка нажата и прошло время дребезга
                button_state = BUTTON_STATE_PRESSED;
                led_task_next_state();
                click_counter++;
            }
            if (level == true){
                button_state = BUTTON_STATE_RELEASED_DEBOUNCE;
                last_toggle_us = now_us; // сбрасываем счетчик времени.
            }
            break;
    }
}