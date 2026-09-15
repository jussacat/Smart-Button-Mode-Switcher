#ifndef BUTTON_H
#define BUTTON_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_22

typedef enum {
    BTN_EVT_CLICK = 1,
    BTN_EVT_LONGPRESS, //>2s
    BTN_EVT_TRIGGER_HANG //>5s watchdog
} button_event_t;

extern QueueHandle_t button_queue;

void Button_Init(void);

#endif /*_BUTTON_H_*/