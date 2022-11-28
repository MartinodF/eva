#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "eva_led.h"
#include "eva_leds.h"
#include "eva_sntp.h"
#include "eva_wifi.h"

#define EVA_STATUS_BRIGHTNESS 0.10f;
#define EVA_STATUS_GPIO GPIO_NUM_40

void status_loop(void*);
