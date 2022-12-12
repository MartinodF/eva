/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>

#include "driver/rmt_encoder.h"
#include "esp_attr.h"
#include "esp_check.h"
#include "eva_leds.h"

#define EVA_RMT_SK6812RGBW_T0H_US (0.3)
#define EVA_RMT_SK6812RGBW_T0L_US (0.9)
#define EVA_RMT_SK6812RGBW_T1H_US (0.6)
#define EVA_RMT_SK6812RGBW_T1L_US (0.6)
#define EVA_RMT_SK6812RGBW_RESET_US (80)

typedef struct {
  rmt_encoder_t combo;
  rmt_encoder_t *bytes;
  rmt_encoder_t *copy;
  int state;
  rmt_symbol_word_t reset_code;
} rmt_combo_encoder_t;

esp_err_t rmt_new_encoder(rmt_encoder_handle_t *);
