idf_component_register(SRCS "button_led.c"
        INCLUDE_DIRS "include"
        REQUIRES driver freertos esp_timer
)