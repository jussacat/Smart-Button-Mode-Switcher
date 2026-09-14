#ifndef BUTTON_H
#define BUTTON_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_18

typedef enum {
    BTN_EVT_CLICK = 1
} button_event_t;

extern QueueHandle_t g_button_queue;

void Button_Pressed(void);

#endif /*_BUTTON_H_*/