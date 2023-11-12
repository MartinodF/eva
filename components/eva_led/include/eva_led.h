#pragma once

#include <math.h>
#include <time.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "eva_display.h"
#include "eva_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "led_strip.h"

#define EVA_LED_GPIO_NUM GPIO_NUM_18
#define EVA_LED_RESOLUTION_HZ 10000000UL  // 10MHz resolution, 1 tick = 0.1us
#define EVA_ONBOARD_GPIO_NUM GPIO_NUM_13
#define EVA_LED_FADE_DURATION 25  // ms
#define EVA_LED_INTERVAL 20       // ms

void led_loop(void*);

typedef struct fading {
  uint32_t start;
  int prevW;
  int endW;
  int prevR;
  int endR;
} fading;
