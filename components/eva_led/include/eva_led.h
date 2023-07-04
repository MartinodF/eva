#pragma once

#include <math.h>
#include <time.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "eva_display.h"
#include "eva_event.h"
#include "eva_leds.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define EVA_LED_COLORS EVA_RMT_BYTES_PER_LED
#define EVA_LED_GPIO_NUM GPIO_NUM_18
#define EVA_ONBOARD_GPIO_NUM GPIO_NUM_13
#define EVA_LED_FADE_DURATION 25  // ms
#define EVA_LED_INTERVAL 20       // ms

void led_loop(void*);

typedef struct fading {
  uint32_t start;
  int prev;
  int end;
} fading;
