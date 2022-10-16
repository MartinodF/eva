#pragma once

#include "esp_log.h"
#include "eva_clock.h"
#include "eva_leds.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define EVA_LED_BRIGHTNESS 0.30f;
#define EVA_LED_COLORS EVA_RMT_BYTES_PER_LED
#define EVA_LED_DEBUG_FRAMES false
#define EVA_LED_GPIO_NUM 18

void led_loop(void*);
