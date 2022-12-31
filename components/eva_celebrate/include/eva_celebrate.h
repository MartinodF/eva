#pragma once

#include <math.h>

#include "esp_log.h"
#include "eva_display.h"
#include "eva_event.h"
#include "eva_strings.h"
#include "time.h"

#define EVA_CELEBRATE_NAME_LENGTH 4

typedef int leds[EVA_CELEBRATE_NAME_LENGTH];

typedef struct celebration {
  const char *id;
  int month;
  int day;
  int name;
  const char *greeting;
  time_t trigger;
} celebration_t;

enum Names {
  Fede,
  Mart,
  June,
  Greg,
  FM,
};

void celebrate_start(void);
