#ifndef APP_TASK_H
#define APP_TASK_H

#include "esp_err.h"

typedef enum {
    STATE_FAST = 0,
    STATE_SLOW,
    STATE_OFF,
    STATE_MAX
} system_state_t;

esp_err_t App_Start(void);

#endif /*APP_TASK_H*/
