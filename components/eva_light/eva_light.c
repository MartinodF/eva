#include "eva_light.h"

static int current = 0;
static int raw[EVA_LIGHT_MEASUREMENTS];
SemaphoreHandle_t light_semaphore = NULL;
StaticSemaphore_t light_semaphore_buffer;

static const char *TAG = "light";

void light_start(void) { light_semaphore = xSemaphoreCreateMutexStatic(&light_semaphore_buffer); }

void light_loop(void *) {
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

  for (;;) {
    xSemaphoreTake(light_semaphore, portMAX_DELAY);
    ESP_ERROR_CHECK(adc_oneshot_read(adc, EVA_LIGHT_ADC_CHANNEL, &raw[current]));
    xSemaphoreGive(light_semaphore);

    if (!current) {
      ESP_LOGD(TAG, "level: %d/4095", raw[current]);
    }

    current = (current + 1) % EVA_LIGHT_MEASUREMENTS;

    vTaskDelay(pdMS_TO_TICKS(EVA_LIGHT_INTERVAL));
  }

  ESP_ERROR_CHECK(adc_oneshot_del_unit(adc));

  ESP_LOGW(TAG, "light_loop exited");
}

void light_get(int *brightness) {
  *brightness = 0;

  xSemaphoreTake(light_semaphore, portMAX_DELAY);
  for (int i = 0; i < EVA_LIGHT_MEASUREMENTS; i++) {
    *brightness += raw[i];
  }
  xSemaphoreGive(light_semaphore);

  *brightness /= EVA_LIGHT_MEASUREMENTS;
}
