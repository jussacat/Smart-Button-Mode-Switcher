#include "st7735.h" 
#include "freertos/task.h" 
#include "esp_timer.h"
#include <string.h> 
#include <stdio.h>  
#include "led.h"
 
static spi_device_handle_t s_spi = NULL;  

 
#include "esp_log.h"
static const char *TAG_TFT = "ST7735";

void ST7735_SendCmd(uint8_t cmd) { 
    gpio_set_level(PIN_NUM_DC, 0); 
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data = { cmd }, 
    };  
    spi_device_polling_transmit(s_spi, &t);    
}
 
void ST7735_SendData(const uint8_t *data, int len) {  
    if (len <= 0) return;  
    gpio_set_level(PIN_NUM_DC, 1); 
    spi_transaction_t t = {0};
    if (len <= 4) {
        t.flags = SPI_TRANS_USE_TXDATA;
        t.length = len * 8;
        memcpy(t.tx_data, data, len);
    } else {
        t.length = len * 8; 
        t.tx_buffer = data; 
    }
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
    ['-' - 0x20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
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
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_conf, SPI_DMA_CH_AUTO);  
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_TFT, "spi_bus_initialize failed: %s", esp_err_to_name(ret));
        return;
    }
 
    spi_device_interface_config_t dev_conf = {   
        .clock_speed_hz = 4 * 1000 * 1000, // 4 MHz (tốc độ an toàn, chống nhiễu trên breadboard)
        .mode = 0,  
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,   
        .cs_ena_pretrans = 1,
        .cs_ena_posttrans = 1,
    };  
    ret = spi_bus_add_device(SPI2_HOST, &dev_conf, &s_spi);   
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_TFT, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG_TFT, "SPI bus initialized successfully at 4MHz");   
 
    // Hardware Reset 
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(50)); 
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    //Software Reset    
    ST7735_SendCmd(0x01); 
    vTaskDelay(pdMS_TO_TICKS(150));
 
    //Sleep Out    
    ST7735_SendCmd(0x11); 
    vTaskDelay(pdMS_TO_TICKS(200));
 
    //Frame Rate Control (Normal mode)
    ST7735_SendCmd(0xB1);
    uint8_t frmctr1[] = {0x01, 0x2C, 0x2D};
    ST7735_SendData(frmctr1, sizeof(frmctr1));

    //Frame Rate Control (Idle mode)
    ST7735_SendCmd(0xB2);
    uint8_t frmctr2[] = {0x01, 0x2C, 0x2D};
    ST7735_SendData(frmctr2, sizeof(frmctr2));

    //Frame Rate Control (Partial mode)
    ST7735_SendCmd(0xB3);
    uint8_t frmctr3[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};
    ST7735_SendData(frmctr3, sizeof(frmctr3));

    //Display Inversion Control
    ST7735_SendCmd(0xB4);
    uint8_t invctr = 0x07; // Line inversion
    ST7735_SendData(&invctr, 1);

    //Power Control 1
    ST7735_SendCmd(0xC0);
    uint8_t pwctr1[] = {0xA2, 0x02, 0x84};
    ST7735_SendData(pwctr1, sizeof(pwctr1));

    //Power Control 2
    ST7735_SendCmd(0xC1);
    uint8_t pwctr2 = 0xC5;
    ST7735_SendData(&pwctr2, 1);

    //Power Control 3 (Normal mode)
    ST7735_SendCmd(0xC2);
    uint8_t pwctr3[] = {0x0A, 0x00};
    ST7735_SendData(pwctr3, sizeof(pwctr3));

    //Power Control 4 (Idle mode)
    ST7735_SendCmd(0xC3);
    uint8_t pwctr4[] = {0x8A, 0x2A};
    ST7735_SendData(pwctr4, sizeof(pwctr4));

    //Power Control 5 (Partial mode)
    ST7735_SendCmd(0xC4);
    uint8_t pwctr5[] = {0x8A, 0xEE};
    ST7735_SendData(pwctr5, sizeof(pwctr5));

    //VCOM Control 1
    ST7735_SendCmd(0xC5);
    uint8_t vmctr1 = 0x0E;
    ST7735_SendData(&vmctr1, 1);

    //Display Inversion Off
    ST7735_SendCmd(0x20);

    //Memory Data Access Control (Orientation & BGR/RGB)
    ST7735_SendCmd(0x36);
    uint8_t madctl = 0xC8; // BGR format  
    ST7735_SendData(&madctl, 1);   

    //Color mode 16-bit (RGB565)
    ST7735_SendCmd(0x3A); 
    uint8_t col_mode = 0x05;
    ST7735_SendData(&col_mode, 1); 

    //Gamma positive correction
    ST7735_SendCmd(0xE0);
    uint8_t gmctrp1[] = {
        0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
        0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10
    };
    ST7735_SendData(gmctrp1, sizeof(gmctrp1));

    //Gamma negative correction
    ST7735_SendCmd(0xE1);
    uint8_t gmctrn1[] = {
        0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10
    };
    ST7735_SendData(gmctrn1, sizeof(gmctrn1));

    //Normal Display Mode On
    ST7735_SendCmd(0x13);
    vTaskDelay(pdMS_TO_TICKS(10));

    //Display ON   
    ST7735_SendCmd(0x29);
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
