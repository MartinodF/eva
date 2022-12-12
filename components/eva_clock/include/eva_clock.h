#pragma once

#include <string.h>

#include "esp_log.h"
#include "eva_display.h"
#include "eva_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "time.h"

#define CEILING(x, y) (((x) + (y)-1) / (y))

#define EVA_CLOCK_MASK_SIZE 2
#define EVA_CLOCK_LEDS_SIZE 4
#define EVA_CLOCK_DEBUG false

typedef struct clockmask {
  int offset;
  uint8_t words[EVA_CLOCK_MASK_SIZE];
} clockmask;

typedef int clockleds[EVA_CLOCK_LEDS_SIZE];

enum Words {
  ItIs,
  FiveMinutes,
  TenMinutes,
  AQuarter,
  TwentyMinutes,
  Half,
  OClock,
  To,
  Past,
  Twelve,
  One,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Eleven,
  Fede,
  Mart,
  June,
  Greg,
  FM,
};

void clock_loop(void *);
