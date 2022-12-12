#include "eva_light.h"

static int current = 0;
static int raw[EVA_LIGHT_MEASUREMENTS];

static const char *TAG = "light";

void light_loop(void *unused) {
  ESP_LOGI(TAG, "light_loop starting");

  adc_oneshot_unit_handle_t adc;
  adc_oneshot_unit_init_cfg_t config = {
      .unit_id = EVA_LIGHT_ADC_UNIT,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&config, &adc));

  adc_oneshot_chan_cfg_t chan_config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_6,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, EVA_LIGHT_ADC_CHANNEL, &chan_config));

  int average;

  for (;;) {
    ESP_ERROR_CHECK(adc_oneshot_read(adc, EVA_LIGHT_ADC_CHANNEL, &raw[current]));

    if (abs(raw[current] - average) > EVA_LIGHT_THRESHOLD) {
      average = 0;

      for (int i = 0; i < EVA_LIGHT_MEASUREMENTS; i++) {
        average += raw[i];
      }

      average /= EVA_LIGHT_MEASUREMENTS;

      ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_LIGHT_UPDATE, &average, sizeof(average), portMAX_DELAY));
      ESP_LOGI(TAG, "level: %d/4095", average);
    }

    current = (current + 1) % EVA_LIGHT_MEASUREMENTS;

    vTaskDelay(pdMS_TO_TICKS(EVA_LIGHT_INTERVAL));
  }

  ESP_ERROR_CHECK(adc_oneshot_del_unit(adc));

  ESP_LOGW(TAG, "light_loop exited");
}
