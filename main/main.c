#include "main.h"

static const char* TAG = "main";

void app_main(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_start();
  sntp_start();
  clock_start();

  TaskHandle_t app_handle = NULL;
  TaskHandle_t clock_handle = NULL;
  TaskHandle_t led_handle = NULL;
  xTaskCreate(app_loop, "app_loop", STACK_SIZE, NULL, 1UL, &app_handle);
  configASSERT(app_handle);
  xTaskCreate(clock_loop, "clock_loop", STACK_SIZE, NULL, 1UL, &clock_handle);
  configASSERT(clock_handle);
  xTaskCreate(led_loop, "led_loop", STACK_SIZE, NULL, 1UL, &led_handle);
  configASSERT(led_handle);

#if CONFIG_PM_ENABLE
  // Configure dynamic frequency scaling:
  // maximum and minimum frequencies are set in sdkconfig,
  // automatic light sleep is enabled if tickless idle support is enabled.
  esp_pm_config_esp32s3_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 40,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
    // TODO: Enable light sleep when this bug is fixed
    // https://github.com/espressif/esp-idf/issues/8507
    .light_sleep_enable = false
#endif
  };

  ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif  // CONFIG_PM_ENABLE
}

void app_loop(void* param) {
  for (;;) {
    ESP_LOGI(TAG, "SNTP status: %s", sntp_healthy() ? "healthy" : "unhealthy");
    ESP_LOGI(TAG, "Wifi status: %s", wifi_available() ? "healthy" : "unhealthy");

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
