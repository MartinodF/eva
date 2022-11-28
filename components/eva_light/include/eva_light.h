#pragma once

#include <string.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define EVA_LIGHT_ADC_UNIT ADC_UNIT_1
#define EVA_LIGHT_ADC_CHANNEL ADC_CHANNEL_0
#define EVA_LIGHT_INTERVAL 100  // ms
#define EVA_LIGHT_MEASUREMENTS 30

void light_start(void);
void light_loop(void *);
void light_get(int *);
