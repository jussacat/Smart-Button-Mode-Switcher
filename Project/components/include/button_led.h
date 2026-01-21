#ifndef BUTTON_LED_H
#define BUTTON_LED_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"


#define RED_LED_PIN      GPIO_NUM_21
#define BUTTON_PIN   GPIO_NUM_19
#define GREEN_LED_PIN GPIO_NUM_18
#define STATUS_BLINK_TIME 200       // Nháy nhanh (200ms)

typedef enum {
    MODE_FAST = 0, //100ms
    MODE_SLOW, //1000ms
    MODE_OFF //off
} LedMode;

typedef struct {
    LedMode currentMode;
    uint32_t blinkInterval; //time blink
} AppState;

extern bool g_button_pressed;

void Change_Mode(AppState *ptrState);
void Button_Pressed(void);

#endif //BUTTON_LED_H