#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "led.h"
#include "button.h"

static TimerHandle_t s_led_timer = NULL;
static TimerHandle_t s_heartbeat_timer = NULL;
static uint8_t s_red_led_state = 0;
static uint8_t s_green_led_state = 0;

QueueHandle_t g_display_queue = NULL;

static void led_timer_callback(TimerHandle_t xTimer){
    s_red_led_state = !s_red_led_state;
    gpio_set_level(RED_LED_PIN, s_red_led_state);
}

static void heartbeat_timer_callback(TimerHandle_t xTimer) {
    s_green_led_state = !s_green_led_state;
    gpio_set_level(GREEN_LED_PIN, s_green_led_state);
}

void LED_Init(void) {
    gpio_config_t led_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << RED_LED_PIN) | (1ULL << GREEN_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led_conf);

    //RED LED (200ms/2)
    s_led_timer = xTimerCreate("LedTimer", pdMS_TO_TICKS(100), pdTRUE, NULL, led_timer_callback);

    //GREEN
    s_heartbeat_timer = xTimerCreate("Heartbeat", pdMS_TO_TICKS(500), pdTRUE, NULL, heartbeat_timer_callback);
    xTimerStart(s_heartbeat_timer, 0);
}

void Change_Mode(void *pvParameters){
    LedMode_t current_mode = MODE_FAST;
    uint32_t blink_ms = 200;

    xTimerChangePeriod(s_led_timer, pdMS_TO_TICKS(blink_ms / 2), 0);
    xTimerStart(s_led_timer, 0);

    message_t msg = {current_mode, blink_ms};
    xQueueSend(g_display_queue, &msg, 0);

    button_event_t evt;

    while(1){
        if(xQueueReceive(g_button_queue, &evt, portMAX_DELAY) == pdTRUE){
            switch(current_mode){
                case MODE_FAST:
                    current_mode = MODE_SLOW;
                    blink_ms = 1000;
                    xTimerChangePeriod(s_led_timer, pdMS_TO_TICKS(blink_ms / 2), 0);
                    xTimerStart(s_led_timer, 0);
                    break;
                case MODE_SLOW:
                    current_mode = MODE_OFF;
                    blink_ms = 0;
                    xTimerStop(s_led_timer, 0);
                    gpio_set_level(RED_LED_PIN, 0);
                    break;
                case MODE_OFF:
                    current_mode = MODE_FAST;
                    blink_ms = 200;
                    xTimerChangePeriod(s_led_timer, pdMS_TO_TICKS(blink_ms / 2), 0);
                    xTimerStart(s_led_timer, 0); 
                    break;
            }

            msg.mode = current_mode;
            msg.ms = blink_ms;
            xQueueSend(g_display_queue, &msg, 0);

        }
    }
}
