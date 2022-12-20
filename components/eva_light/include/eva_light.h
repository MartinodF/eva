#pragma once

#include <string.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_event.h"
#include "esp_log.h"
#include "eva_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define EVA_LIGHT_ADC_UNIT ADC_UNIT_1
#define EVA_LIGHT_ADC_CHANNEL ADC_CHANNEL_0
#define EVA_LIGHT_INTERVAL 100  // ms
#define EVA_LIGHT_MIN 15        // measured experimentally
#define EVA_LIGHT_MAX 3230      // measured experimentally
#define EVA_LIGHT_MEASUREMENTS 30
#define EVA_LIGHT_THRESHOLD 1

void light_loop(void *);
