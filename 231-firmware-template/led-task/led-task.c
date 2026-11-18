#include "led-task.h"
#include "log.h"

const uint LED_PIN = 25;

static led_state_t led_state;
static uint32_t half_period_ms = 500; // милисекунды 
static uint64_t last_toggle_us = 0;


static void led_task_blink(uint64_t * last_us, uint32_t half_period_ms);
static void led_set_level(bool level);
static bool led_get_level(void);



static void led_set_level(bool level)
{
    gpio_put(LED_PIN, level);
}
static bool led_get_level(void)
{
    return gpio_get(LED_PIN);
}

static bool period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    uint64_t now_us = time_us_64();

    if (now_us - *last_us < period_us) {
        return false;
    }

    // Следующее срабатывание отсчитывается от прошлого, а не от «сейчас»
    *last_us += period_us;

    // Отстали больше чем на период: не догонять, а начать отсчёт заново
    if (now_us - *last_us >= period_us) {
        *last_us = now_us;
    }

    return true;
}

static void led_task_blink(uint64_t * last_us, uint32_t half_period_ms)
{

    if (period_elapsed(last_us, half_period_ms*1000))
    {
        led_set_level(!led_get_level());
    }
}



void led_task_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    led_task_set_state(LED_STATE_BLINK);
    // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    switch (led_state)
    {
        case LED_STATE_OFF:
        led_set_level(false);
        break;
        case LED_STATE_ON:
        led_set_level(true);
        break;
        case LED_STATE_BLINK:
        led_set_level(!led_get_level());
        break;
        default:
        LOG_DBG("An led was made to establish a non-existent state of the led");
        break;
    }
}
led_state_t led_task_get_state(void){
    return led_state;
}



void led_task_handle(void)
{
    switch (led_state)
    {
        case LED_STATE_OFF:
        break;
        case LED_STATE_ON:
        break;
        case LED_STATE_BLINK:
        led_task_blink( &last_toggle_us, half_period_ms );
        break;
    }
}

bool led_task_set_period_ms(uint32_t period_ms){
    if (period_ms == 0) {
        return false;
    }

    half_period_ms = period_ms / 2;
    return true;
}
uint32_t led_task_get_period_ms(void){
    return half_period_ms*2;
}

void print_led_state(void){
    const char *name_state;
    switch (led_state)
    {
    case LED_STATE_OFF:
        name_state = "disable";
        break;
    case LED_STATE_ON:
        name_state = "enable";
        break;
    default:
        name_state = "blink";
        break;
    }

    printf("led: %s, period %lu ms \r\n", name_state, half_period_ms*2);
}


