#include "button_led.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"


bool g_button_pressed = false;


void Change_Mode(AppState *ptrState){
    switch(ptrState->currentMode){
        case MODE_FAST:
            ptrState->currentMode = MODE_SLOW;
            ptrState->blinkInterval = 2000;
            printf("Chuyen sang: SLOW (1000ms)\n");
            break;
        case MODE_SLOW:
            ptrState->currentMode = MODE_OFF;
            printf("Chuyen sang: OFF\n");
            break;
        case MODE_OFF:
            ptrState->currentMode = MODE_FAST;
            ptrState->blinkInterval = 500;
            printf("Chuyen sang: FAST\n");
            break;
    }
}

static void IRAM_ATTR gpio_isr_handler(void *arg){
    static uint64_t last_isr_time = 0;
    //get current time (us)
    uint64_t current_time = esp_timer_get_time(); 
    //DEBOUNCE
    //200ms = 200,000us 
    if((current_time - last_isr_time) > 200000){
        last_isr_time = current_time;
        //Flag
        g_button_pressed = true;
    } 

}

void Button_Pressed(void){

    g_button_pressed = true;
    
    gpio_config_t led_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << RED_LED_PIN) | (1ULL << GREEN_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led_conf);

    gpio_config_t button_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 1,    
    };
    gpio_config(&button_conf);

    //cài ngắt
    gpio_install_isr_service(0); //mặc định
    gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handler, NULL); //gắn vào chân button

    //first state
    AppState mySystemState = {MODE_FAST, 500};

    uint64_t last_blink_time = 0;
    int led_state = 0;

    uint64_t last_blink_time2 = 0;
    int green_state = 0;

    printf("System started. Press button to change mode.\n");

    while (1){
        if(g_button_pressed){
            Change_Mode(&mySystemState);
            g_button_pressed = false;
        }
        if(mySystemState.currentMode != MODE_OFF){
            uint64_t now = esp_timer_get_time() / 1000;
            if(now - last_blink_time >= mySystemState.blinkInterval){
                last_blink_time = now;
                led_state = !led_state;
                gpio_set_level(RED_LED_PIN, led_state);
            }
            if(now - last_blink_time2 >= STATUS_BLINK_TIME){
                last_blink_time2 = now;
                green_state = !green_state;
                gpio_set_level(GREEN_LED_PIN, green_state);
            }
        }
        else{
                gpio_set_level(RED_LED_PIN, 1);
                gpio_set_level(GREEN_LED_PIN, 0);
            }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}