#pragma once

#include "driver/touch_pad.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "eva_display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define EVA_TOUCH_TOUCH_NUM TOUCH_PAD_NUM5
#define EVA_TOUCH_DELAY 250      // ms
#define EVA_TOUCH_THRESHOLD 0.2  // 20%

void touch_loop(void *unused);
