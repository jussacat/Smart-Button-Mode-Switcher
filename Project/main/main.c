#include "esp_log.h" 
#include "app_task.h"

        
static const char *MAIN_TAG = "MAIN";
void app_main(void)  
{       
    ESP_LOGI(MAIN_TAG, "Smart Button System Initializing...");

    if(App_Start() != ESP_OK){
        ESP_LOGE(MAIN_TAG, "Failed to start FSM Application");
    }
}

