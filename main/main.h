#pragma once

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "eva_clock.h"
#include "eva_debug.h"
#include "eva_light.h"
#include "eva_sntp.h"
#include "eva_status.h"
#include "eva_strings.h"
#include "eva_temp.h"
#include "eva_touch.h"
#include "eva_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#define STARTUP_TIME 5000ULL  // 5 seconds
#define STACK_SIZE 4096

void app_main(void);
void app_loop(void*);
