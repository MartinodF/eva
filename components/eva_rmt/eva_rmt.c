#include "eva_rmt.h"

#include <stdio.h>

void rmt_start(void) {
  rmt_config_t config = {.rmt_mode = RMT_MODE_TX,
                         .channel = RMT_CHANNEL_0,
                         .gpio_num = 1,
                         .clk_div = 8,
                         .mem_block_num = 4,
                         .flags = RMT_CHANNEL_FLAGS_AWARE_DFS,
                         .tx_config = {
                             .idle_level = RMT_IDLE_LEVEL_LOW,
                             .carrier_en = false,
                             .loop_en = false,
                             .idle_output_en = true,
                         }};

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}
