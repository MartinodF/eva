#include "eva_leds.h"

static const char *TAG = "leds";

esp_err_t rmt_leds_send(rmt_leds_t *leds) {
  ESP_RETURN_ON_ERROR(rmt_enable(leds->channel), TAG, "failed to enable RMT channel");

  ESP_RETURN_ON_ERROR(
      rmt_transmit(leds->channel, leds->encoder, leds->pixels, leds->count * EVA_RMT_BYTES_PER_LED, &leds->tx_config),
      TAG, "failed to transmit RMT");

  ESP_RETURN_ON_ERROR(rmt_tx_wait_all_done(leds->channel, 50), TAG, "error while waiting for RMT TX to finish");

  ESP_RETURN_ON_ERROR(rmt_disable(leds->channel), TAG, "failed to disable RMT channel");

  return ESP_OK;
}

esp_err_t rmt_leds_del(rmt_leds_t *leds) {
  rmt_del_channel(leds->channel);
  rmt_del_encoder(leds->encoder);
  free(leds);
  return ESP_OK;
}

esp_err_t rmt_new_leds(int gpio, int count, const void *pixels, rmt_leds_handle_t *ret_leds) {
  esp_err_t ret = ESP_OK;

  ESP_LOGI(TAG, "rmt_new_leds (gpio: %d, count: %d)", gpio, count);

  rmt_leds_t *leds = NULL;
  leds = calloc(1, sizeof(rmt_leds_t));
  ESP_GOTO_ON_FALSE(leds, ESP_ERR_NO_MEM, err, TAG, "failed to allocate RMT leds");

  int symbols = (count >= 48) ? ((count % 2 == 0) ? count : count + 1) : 48;

  rmt_tx_channel_config_t config = {
      .clk_src = RMT_CLK_SRC_XTAL,
      .gpio_num = gpio,
      .mem_block_symbols = symbols,  // symbols * 4 bytes
      .resolution_hz = EVA_RMT_RESOLUTION_HZ,
      .trans_queue_depth = 16,
  };
  ESP_GOTO_ON_ERROR(rmt_new_tx_channel(&config, &leds->channel), err, TAG, "failed to create RMT TX channel");

  ESP_GOTO_ON_ERROR(rmt_new_encoder(&leds->encoder), err, TAG, "failed to create RMT encoder");

  leds->count = count;
  leds->pixels = pixels;
  leds->tx_config = (rmt_transmit_config_t){
      .loop_count = 0,
  };

  *ret_leds = leds;

  return ESP_OK;

err:
  if (leds) {
    if (leds->channel) {
      rmt_del_channel(leds->channel);
    }
    if (leds->encoder) {
      rmt_del_encoder(leds->encoder);
    }
    free(leds);
  }
  return ret;
}
