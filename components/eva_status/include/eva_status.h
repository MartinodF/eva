#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "eva_event.h"
#include "eva_led.h"

#define EVA_STATUS_BRIGHTNESS 0.10f;
#define EVA_STATUS_GPIO GPIO_NUM_40

enum Statuses { WiFi, SNTP, StatusesCount };

void status_start(void);
