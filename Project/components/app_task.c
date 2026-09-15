#include "app_task.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "button.h"
#include "led.h"
#include "web_server.h"


static const char *APP_TAG = "APP_FSM";


static const char* state_to_name(system_state_t state){
    switch(state) {
        case STATE_FAST: return "MODE_FAST (200ms)";
        case STATE_SLOW: return "MODE_SLOW (1000ms)";
        case STATE_OFF: return "MODE_OFF";
        default: return "UNKNOWN";
    }
}

static void FSM_Task(void *pvParameters){
    system_state_t current_state = STATE_FAST;
    button_event_t evt;

    esp_task_wdt_add(NULL);
    ESP_LOGI(APP_TAG, "Task FSM subscribed to Task Watchdog Timer (TWDT)");

    LED_SetRate(200);
    ESP_LOGI(APP_TAG, "[INIT STATE] Current state: %s", state_to_name(current_state));

    while(1){
        esp_task_wdt_reset();

        if(xQueueReceive(button_queue, &evt, pdMS_TO_TICKS(1000)) == pdTRUE){
            if(evt == BTN_EVT_CLICK){
                current_state = (system_state_t)((current_state + 1) % STATE_MAX);
                ESP_LOGW(APP_TAG, "[EVENT] Button Pressed! Transition to -> %s", state_to_name(current_state));

                switch (current_state) {
                    case STATE_FAST:
                        LED_SetRate(200);
                        break;
                    case STATE_SLOW:
                        LED_SetRate(1000);
                        break;
                    case STATE_OFF:
                        LED_SetStatusOff();
                        break;
                    default:
                        break;
                }

                ESP_LOGI(APP_TAG, "[TELEMETRY] Free Heap: %lu B | Stack High Watermark: %u words", 
                    (unsigned long)esp_get_free_heap_size(),
                    (unsigned int)uxTaskGetStackHighWaterMark(NULL)); 

            }
            else if (evt == BTN_EVT_TRIGGER_HANG) {
                ESP_LOGE(APP_TAG, "[TRIGGER PRESS DETECTED] TASK FROZEN!");
                ESP_LOGE(APP_TAG, "Watchdog is about to reset chip...");

                while(1){}
            }
        }
    }
}

esp_err_t App_Start(void){
    esp_task_wdt_config_t twdt_conf = {
        .timeout_ms = 3000,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_task_wdt_reconfigure(&twdt_conf);

    Button_Init();
    LED_Init();

    WebServer_Init();

    BaseType_t ret = xTaskCreate(FSM_Task, "FSM_Task", 4096, NULL, 5, NULL);

    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}