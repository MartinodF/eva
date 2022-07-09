#pragma once

#include "driver/rmt_tx.h"
#include "led_strip.h"

#define RMT_SK6812RGBW_T0H_NS (300)
#define RMT_SK6812RGBW_T0L_NS (900)
#define RMT_SK6812RGBW_T1H_NS (600)
#define RMT_SK6812RGBW_T1L_NS (600)
#define RMT_SK6812RGBW_RESET_US (80)

void rmt_start(void);
