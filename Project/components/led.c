#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static TimerHandle_t status_timer = NULL;
static TimerHandle_t heartbeat_timer = NULL;
static uint8_t status_led_state = 0;
static uint8_t heartbeat_led_state = 0;

QueueHandle_t g_display_queue = NULL;

static void status_timer_callback(TimerHandle_t xTimer){
    status_led_state = !status_led_state;
    gpio_set_level(RED_LED_PIN, status_led_state);
}

static void heartbeat_timer_callback(TimerHandle_t xTimer) {
    heartbeat_led_state = !heartbeat_led_state;
    gpio_set_level(GREEN_LED_PIN, heartbeat_led_state);
}

void LED_Init(void) {
    gpio_config_t led_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << RED_LED_PIN) | (1ULL << GREEN_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led_conf);

    //RED(200ms/2)
    status_timer = xTimerCreate("StatusLed", pdMS_TO_TICKS(100), pdTRUE, NULL, status_timer_callback);

    //GREEN
    heartbeat_timer = xTimerCreate("Heartbeat", pdMS_TO_TICKS(500), pdTRUE, NULL, heartbeat_timer_callback);
    xTimerStart(heartbeat_timer, 0);
}

void LED_SetRate(uint32_t period_ms){
    xTimerChangePeriod(status_timer, pdMS_TO_TICKS(period_ms / 2), 0);
    xTimerStart(status_timer, 0);
}

void LED_SetStatusOff(void){
    xTimerStop(status_timer, 0);
    gpio_set_level(RED_LED_PIN, 0);
}
