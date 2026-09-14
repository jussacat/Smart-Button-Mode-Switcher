#ifndef LED_H
#define LED_H

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define RED_LED_PIN      GPIO_NUM_19
#define GREEN_LED_PIN    GPIO_NUM_23

typedef enum {
    MODE_FAST = 0, //200ms
    MODE_SLOW, //1000ms
    MODE_OFF //off
} LedMode_t;

typedef struct {
    LedMode_t mode;
    uint32_t ms;
} message_t;

extern QueueHandle_t g_display_queue;

void LED_Init(void);

void Change_Mode(void *pvParameters);

#endif /*LED_H*/

