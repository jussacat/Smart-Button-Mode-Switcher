#include "button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"

QueueHandle_t g_button_queue = NULL;

static void IRAM_ATTR button_isr_handler(void *arg) {
    static uint64_t last_isr_time = 0;
    //get current time (us)
    uint64_t current_time = esp_timer_get_time();

    //Debounce
    if((current_time - last_isr_time) > 200000){
        last_isr_time = current_time;
        button_event_t evt = BTN_EVT_CLICK;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        //Send event from ISR to queue
        xQueueSendFromISR(g_button_queue, &evt, &xHigherPriorityTaskWoken);

        if(xHigherPriorityTaskWoken){
            portYIELD_FROM_ISR();
        }
    } 
}

void Button_Pressed(void){
    g_button_queue = xQueueCreate(5, sizeof(button_event_t));

    gpio_config_t button_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 1,    
    };
    gpio_config(&button_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
}

