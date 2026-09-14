#ifndef ST7735_H
#define ST7735_H
    
#include <stdint.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"    
#include "freertos/FreeRTOS.h" 
#include "freertos/queue.h"

#define ST7735_WIDTH 128
#define ST7735_HEIGHT 160

#define COLOR_BLACK    0x0000
#define COLOR_WHITE    0xFFFF
#define COLOR_RED      0xF800
#define COLOR_GREEN    0x07E0
#define COLOR_BLUE     0x001F
#define COLOR_YELLOW   0xFFE0
#define COLOR_CYAN     0x07FF
#define COLOR_GRAY     0x8410       


#define PIN_NUM_MISO  -1
#define PIN_NUM_MOSI  GPIO_NUM_22
#define PIN_NUM_CLK   GPIO_NUM_21
#define PIN_NUM_CS    GPIO_NUM_16
#define PIN_NUM_DC    GPIO_NUM_17
#define PIN_NUM_RST   GPIO_NUM_4


void ST7735_Init(void);
void ST7735_SendCmd(uint8_t cmd);
void ST7735_SendData(const uint8_t *data, int len);          
void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);             
void ST7735_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);         
void ST7735_DrawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg_color);    
void ST7735_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg_color);

void ST7735_DisplayTask(void *pvParameters);
#endif /*ST7735_H*/