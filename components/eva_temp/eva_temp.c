#include "eva_temp.h"

static int current = 0;
static float raw[EVA_TEMP_MEASUREMENTS];

static const char* TAG = "temp";

void temp_loop(void* unused) {
  ESP_LOGI(TAG, "temp_loop starting");

  temperature_sensor_handle_t temp_handle = NULL;
  temperature_sensor_config_t temp_sensor = TEMPERATURE_SENSOR_CONFIG_DEFAULT(0, 60);
  ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));

  float average;

  for (;;) {
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &raw[current]));
    ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));

    if (fabs(raw[current] - average) > EVA_TEMP_THRESHOLD) {
      average = 0.0f;

      for (int i = 0; i < EVA_TEMP_MEASUREMENTS; i++) {
        average += raw[i];
      }

      average /= EVA_TEMP_MEASUREMENTS;

      ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_TEMP_UPDATE, &average, sizeof(average), portMAX_DELAY));
      ESP_LOGI(TAG, "average: %.2f °C", average);
    }

    current = (current + 1) % EVA_TEMP_MEASUREMENTS;

    vTaskDelay(pdMS_TO_TICKS(EVA_TEMP_INTERVAL));
  }

  ESP_ERROR_CHECK(temperature_sensor_uninstall(temp_handle));

  ESP_LOGW(TAG, "temp_loop exited");
}
