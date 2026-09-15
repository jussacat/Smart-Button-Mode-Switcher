#include "button.h"
#include "esp_timer.h"

QueueHandle_t button_queue = NULL;

static void IRAM_ATTR button_isr_handler(void *arg) {
    static uint64_t press_time = 0;
    //get current time (us)
    uint64_t current_time = esp_timer_get_time();
    int pin_level = gpio_get_level(BUTTON_PIN);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(pin_level == 0){

        press_time = current_time;

    } else {

        uint64_t duration = (current_time - press_time) / 1000;

        if(duration > 180){
            button_event_t evt;
            if(duration >= 3000){
                evt = BTN_EVT_TRIGGER_HANG;
            } else {
                evt = BTN_EVT_CLICK;
            }

            xQueueSendFromISR(button_queue, &evt, &xHigherPriorityTaskWoken);

            if(xHigherPriorityTaskWoken){
                portYIELD_FROM_ISR();
            }
        }
    }
}

void Button_Init(void){
    button_queue = xQueueCreate(5, sizeof(button_event_t));

    gpio_config_t button_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 1,    
    };
    gpio_config(&button_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
}

