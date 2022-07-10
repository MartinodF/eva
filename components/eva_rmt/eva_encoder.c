/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "eva_encoder.h"

static const char *TAG = "encoder";

static size_t rmt_encode(rmt_encoder_t *sub_encoder, rmt_channel_handle_t channel, const void *primary_data,
                         size_t data_size, rmt_encode_state_t *ret_state) {
  rmt_combo_encoder_t *encoder = __containerof(sub_encoder, rmt_combo_encoder_t, combo);
  rmt_encoder_handle_t bytes = encoder->bytes;
  rmt_encoder_handle_t copy = encoder->copy;

  rmt_encode_state_t session_state = 0;
  rmt_encode_state_t state = 0;

  size_t encoded_symbols = 0;

  switch (encoder->state) {
    case 0:  // send RGB data
      encoded_symbols += bytes->encode(bytes, channel, primary_data, data_size, &session_state);

      if (session_state & RMT_ENCODING_COMPLETE) {
        encoder->state = 1;
      }

      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }

      // fall-through
    case 1:  // send reset code
      encoded_symbols += copy->encode(copy, channel, &encoder->reset_code, sizeof(encoder->reset_code), &session_state);

      if (session_state & RMT_ENCODING_COMPLETE) {
        encoder->state = 0;
        state |= RMT_ENCODING_COMPLETE;
      }

      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
  }
out:
  *ret_state = state;
  return encoded_symbols;
}

static esp_err_t rmt_del(rmt_encoder_t *sub_encoder) {
  rmt_combo_encoder_t *encoder = __containerof(sub_encoder, rmt_combo_encoder_t, combo);
  rmt_del_encoder(encoder->bytes);
  rmt_del_encoder(encoder->copy);
  free(encoder);
  return ESP_OK;
}

static esp_err_t rmt_reset(rmt_encoder_t *sub_encoder) {
  rmt_combo_encoder_t *encoder = __containerof(sub_encoder, rmt_combo_encoder_t, combo);
  rmt_encoder_reset(encoder->bytes);
  rmt_encoder_reset(encoder->copy);
  encoder->state = 0;
  return ESP_OK;
}

esp_err_t rmt_new_encoder(rmt_encoder_handle_t *combo) {
  esp_err_t ret = ESP_OK;

  rmt_combo_encoder_t *encoder = NULL;
  encoder = calloc(1, sizeof(rmt_combo_encoder_t));
  ESP_GOTO_ON_FALSE(encoder, ESP_ERR_NO_MEM, err, TAG, "failed to allocate combo encoder");

  encoder->combo.encode = rmt_encode;
  encoder->combo.del = rmt_del;
  encoder->combo.reset = rmt_reset;

  uint32_t reset_ticks = EVA_RMT_SK6812RGBW_RESET_US * EVA_RMT_RESOLUTION_HZ / 1000000 / 2;

  encoder->reset_code = (rmt_symbol_word_t){
      .level0 = 0,
      .duration0 = reset_ticks,
      .level1 = 0,
      .duration1 = reset_ticks,
  };

  rmt_bytes_encoder_config_t bytes_config = {
      .bit0 = {.level0 = 1,
               .duration0 = EVA_RMT_SK6812RGBW_T0H_US * EVA_RMT_RESOLUTION_HZ / 1000000,
               .level1 = 0,
               .duration1 = EVA_RMT_SK6812RGBW_T0L_US * EVA_RMT_RESOLUTION_HZ / 1000000},
      .bit1 = {.level0 = 1,
               .duration0 = EVA_RMT_SK6812RGBW_T1H_US * EVA_RMT_RESOLUTION_HZ / 1000000,
               .level1 = 0,
               .duration1 = EVA_RMT_SK6812RGBW_T1L_US * EVA_RMT_RESOLUTION_HZ / 1000000},
      .flags.msb_first = 1};

  rmt_copy_encoder_config_t copy_config = {};

  ESP_GOTO_ON_ERROR(rmt_new_bytes_encoder(&bytes_config, &encoder->bytes), err, TAG, "failed to create bytes encoder");
  ESP_GOTO_ON_ERROR(rmt_new_copy_encoder(&copy_config, &encoder->copy), err, TAG, "failed to create copy encoder");

  *combo = &encoder->combo;

  return ESP_OK;

err:
  if (encoder) {
    if (encoder->bytes) {
      rmt_del_encoder(encoder->bytes);
    }
    if (encoder->copy) {
      rmt_del_encoder(encoder->copy);
    }
    free(encoder);
  }
  return ret;
}
