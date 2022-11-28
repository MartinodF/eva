#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define EVA_DEBUG_TICKS 5000
#define EVA_DEBUG_ARRAY_SIZE_OFFSET 5  // Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE

void debug_loop(void*);
