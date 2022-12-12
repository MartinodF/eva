#pragma once

#include <math.h>

#include "driver/temperature_sensor.h"
#include "esp_event.h"
#include "esp_log.h"
#include "eva_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define EVA_TEMP_INTERVAL 1000  // ms
#define EVA_TEMP_MEASUREMENTS 5
#define EVA_TEMP_THRESHOLD 0.8f

void temp_loop(void* unused);
