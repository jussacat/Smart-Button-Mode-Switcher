idf_component_register(SRCS "button.c" "led.c" "st7735.c"
        INCLUDE_DIRS "include"
        REQUIRES driver esp_driver_gpio esp_driver_spi freertos esp_timer log
)