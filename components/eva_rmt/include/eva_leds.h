#pragma once

#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "eva_encoder.h"

#define EVA_RMT_BYTES_PER_LED 4
#define EVA_RMT_RESOLUTION_HZ 20000000UL  // 20MHz resolution, 1 tick = 0.05us

typedef struct rmt_leds_t {
  rmt_channel_handle_t channel;
  rmt_encoder_handle_t encoder;
  rmt_transmit_config_t tx_config;
  int count;
  const void *pixels;
} rmt_leds_t;

typedef struct rmt_leds_t *rmt_leds_handle_t;

esp_err_t rmt_leds_send(rmt_leds_t *);
esp_err_t rmt_leds_del(rmt_leds_t *);
esp_err_t rmt_new_leds(int, int, const void *, rmt_leds_handle_t *);
