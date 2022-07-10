#include "main.h"

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

  TaskHandle_t clock_handle = NULL;
  TaskHandle_t led_handle = NULL;
  TaskHandle_t status_handle = NULL;
  xTaskCreate(clock_loop, "clock_loop", STACK_SIZE, NULL, 1UL, &clock_handle);
  configASSERT(clock_handle);
  xTaskCreate(led_loop, "led_loop", STACK_SIZE, NULL, 2UL, &led_handle);
  configASSERT(led_handle);
  xTaskCreate(status_loop, "status_loop", STACK_SIZE, NULL, 1UL, &status_handle);
  configASSERT(status_handle);
}
