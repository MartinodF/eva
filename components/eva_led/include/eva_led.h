#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "eva_clock.h"
#include "eva_leds.h"
#include "eva_light.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define EVA_LED_COLORS EVA_RMT_BYTES_PER_LED
#define EVA_LED_DEBUG_FRAMES false
#define EVA_LED_GPIO_NUM GPIO_NUM_18
#define EVA_ONBOARD_GPIO_NUM GPIO_NUM_13
#define EVA_LED_INTERVAL 60  // ms

void led_loop(void*);
