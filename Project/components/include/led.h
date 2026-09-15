#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "driver/gpio.h"

#define RED_LED_PIN      GPIO_NUM_19 //Status
#define GREEN_LED_PIN    GPIO_NUM_21 //Heartbeat


void LED_Init(void);
void LED_SetRate(uint32_t period_ms);
void LED_SetStatusOff(void);

#endif /*LED_H*/

