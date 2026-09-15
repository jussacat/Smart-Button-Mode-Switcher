idf_component_register(SRCS "button.c" "led.c" "app_task.c" "web_server.c"
        INCLUDE_DIRS "include"
        REQUIRES driver esp_driver_gpio esp_system freertos esp_timer log esp_wifi esp_event nvs_flash
        esp_http_server esp_netif
)