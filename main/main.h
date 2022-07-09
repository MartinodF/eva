#include "esp32s3/pm.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "eva_clock.h"
#include "eva_led.h"
#include "eva_sntp.h"
#include "eva_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#define STACK_SIZE 2048

void app_main(void);
void app_loop(void*);
