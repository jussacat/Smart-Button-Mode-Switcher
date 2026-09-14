#include "st7735.h" 
#include "freertos/task.h" 
#include "esp_timer.h"
#include <string.h> 
#include <stdio.h>  
#include "led.h"
 
static spi_device_handle_t s_spi = NULL;  

 
void ST7735_SendCmd(uint8_t cmd) { 
    gpio_set_level(PIN_NUM_DC, 0); 
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd, 
    };  
    spi_device_polling_transmit(s_spi, &t);    
}
 
void ST7735_SendData(const uint8_t *data, int len) {  
    if (len <= 0) return;  
    gpio_set_level(PIN_NUM_DC, 1); 
    spi_transaction_t t = {
        .length = len * 8, 
        .tx_buffer = data, 
    };  
    spi_device_polling_transmit(s_spi, &t);    
}
 
void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
     
    ST7735_SendCmd(0x2A);  
    uint8_t data_x[] = {0x00, x0, 0x00, x1};   
    ST7735_SendData(data_x, sizeof(data_x));   

    ST7735_SendCmd(0x2B);  
    uint8_t data_y[] = {0x00, y0, 0x00, y1};   
    ST7735_SendData(data_y, sizeof(data_y));   
 
    ST7735_SendCmd(0x2C);  
}
 
void ST7735_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT)) return;  
    if ((x + w - 1) >= ST7735_WIDTH)  w = ST7735_WIDTH - x;   
    if ((y + h - 1) >= ST7735_HEIGHT) h = ST7735_HEIGHT - y;  
 
    ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1); 
 
    uint8_t color_buff[64 * 2];
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    for (int i = 0; i < 64; i++) { 
        color_buff[i * 2]= hi; 
        color_buff[i * 2 + 1] = lo; 
    }   
 
    int total_pixels = w * h; 
    while (total_pixels > 0) {
        int batch = (total_pixels > 64) ? 64 : total_pixels;  
        ST7735_SendData(color_buff, batch * 2); 
        total_pixels -= batch;
    }   
}
 
static const uint8_t font8x16_basic[][16] = {  
    [' ' - 0x20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    [':' - 0x20] = {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00,0x00,0x00,0x00,0x00},
    ['-'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['0' - 0x20] = {0x00,0x00,0x18,0x3C,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00},
    ['1' - 0x20] = {0x00,0x00,0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x7E,0x00,0x00},
    ['2' - 0x20] = {0x00,0x00,0x3C,0x66,0x06,0x06,0x0C,0x18,0x30,0x60,0x66,0x7E,0x7E,0x00,0x00,0x00},
    ['5' - 0x20] = {0x00,0x00,0x7E,0x60,0x60,0x7C,0x66,0x06,0x06,0x06,0x66,0x3C,0x18,0x00,0x00,0x00},
    ['A' - 0x20] = {0x00,0x00,0x18,0x3C,0x66,0x66,0x66,0x7E,0x7E,0x66,0x66,0x66,0x66,0x00,0x00,0x00},
    ['F' - 0x20] = {0x00,0x00,0x7E,0x60,0x60,0x7C,0x7C,0x60,0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00},
    ['L' - 0x20] = {0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x7E,0x00,0x00,0x00},
    ['M' - 0x20] = {0x00,0x00,0x66,0x7E,0x7E,0x5A,0x5A,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,0x00},
    ['O' - 0x20] = {0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00},
    ['S' - 0x20] = {0x00,0x00,0x3C,0x66,0x60,0x30,0x18,0x0C,0x06,0x66,0x66,0x3C,0x18,0x00,0x00,0x00},
    ['T' - 0x20] = {0x00,0x00,0x7E,0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00},
    ['W' - 0x20] = {0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x5A,0x5A,0x7E,0x7E,0x66,0x00,0x00,0x00,0x00},
}; 
 
void ST7735_DrawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg_color) {  
    if (c < ' ' || c > 'Z') c = ' '; 
    const uint8_t *bitmap = font8x16_basic[c - ' '];  
 
    ST7735_SetAddressWindow(x, y, x + 7, y + 15);
    uint8_t buffer[8 * 16 * 2];    
    int idx = 0;    
 
    for (int row = 0; row < 16; row++) {  
        uint8_t line = bitmap[row];
        for (int col = 7; col >= 0; col--) {   
            uint16_t pixel_color = (line & (1 << col)) ? color : bg_color;
            buffer[idx++] = pixel_color >> 8;  
            buffer[idx++] = pixel_color & 0xFF;
        } 
    }   
    ST7735_SendData(buffer, sizeof(buffer));   
}
 
void ST7735_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg_color) {   
    while (*str) {  
        ST7735_DrawChar(x, y, *str, color, bg_color); 
        x += 8;
        str++; 
    }   
}
 
void ST7735_Init(void) {   

    gpio_config_t io_conf = { 
        .pin_bit_mask = (1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_RST),
        .mode = GPIO_MODE_OUTPUT,  
    };  
    gpio_config(&io_conf); 
    
    spi_bus_config_t bus_conf = {    
        .miso_io_num = -1, 
        .mosi_io_num = PIN_NUM_MOSI, 
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 128 * 160 * 2 + 8,  
    };  
    spi_bus_initialize(SPI2_HOST, &bus_conf, SPI_DMA_CH_AUTO);  
 
    spi_device_interface_config_t dev_conf = {   
        .clock_speed_hz = 20 * 1000 * 1000, // 20 MHz 
        .mode = 0,  
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,   
    };  
    spi_bus_add_device(SPI2_HOST, &dev_conf, &s_spi);   
 
    // Hardware Reset 
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(50)); 
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    ST7735_SendCmd(0x01); // Software Reset    
    vTaskDelay(pdMS_TO_TICKS(150));
 
    ST7735_SendCmd(0x11); // Sleep Out    
    vTaskDelay(pdMS_TO_TICKS(120));
 
    ST7735_SendCmd(0x3A); // Color mode 16-bit 
    uint8_t col_mode = 0x05;
    ST7735_SendData(&col_mode, 1); 
 
    ST7735_SendCmd(0x36); // Memory Data Access Control 
    uint8_t madctl = 0xC8; // BGR format  
    ST7735_SendData(&madctl, 1);   
 
    ST7735_SendCmd(0x29); // Display ON   
    vTaskDelay(pdMS_TO_TICKS(100));
    
    //Clear Screen
    ST7735_FillRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, COLOR_BLACK); 
}
 
extern QueueHandle_t g_display_queue;
 
void ST7735_DisplayTask(void *pvParameters) {  
    ST7735_Init();  
 
    ST7735_FillRect(0, 0, 128, 25, COLOR_BLUE);
    ST7735_DrawString(16, 5, "SMART RTOS", COLOR_WHITE, COLOR_BLUE);

    message_t msg;
    while (1) {
        if (xQueueReceive(g_display_queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (msg.mode == MODE_FAST) {
                ST7735_FillRect(10, 45, 108, 30, COLOR_GREEN);
                ST7735_DrawString(20, 52, "MODE: FAST", COLOR_BLACK, COLOR_GREEN);
            } else if (msg.mode == MODE_SLOW) {
                ST7735_FillRect(10, 45, 108, 30, COLOR_YELLOW);
                ST7735_DrawString(20, 52, "MODE: SLOW", COLOR_BLACK, COLOR_YELLOW);
            } else {
                ST7735_FillRect(10, 45, 108, 30, COLOR_RED);
                ST7735_DrawString(24, 52, "MODE: OFF ", COLOR_WHITE, COLOR_RED);
            }

            char str_ms[20];
            snprintf(str_ms, sizeof(str_ms), "MS: %lu  ", msg.ms);
            ST7735_DrawString(15, 95, str_ms, COLOR_WHITE, COLOR_BLACK);
        }
    }
}
