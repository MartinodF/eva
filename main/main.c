#include "main.h"

static esp_timer_handle_t startup_timer;

static const char* TAG = "main";

static void handle_touch(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_id == EVA_TOUCH_TRIPLETAP) {
    esp_restart();
  }
}

static void startup_timer_cb(void* param) {
  ESP_LOGI(TAG, "startup_timer triggered");
  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_BOOTED, NULL, 0, portMAX_DELAY));
}

void app_main(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  const esp_timer_create_args_t timer_args = {.callback = &startup_timer_cb, .name = "startup-done"};
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &startup_timer));
  ESP_ERROR_CHECK(esp_timer_start_once(startup_timer, STARTUP_TIME * 1000));

#if CONFIG_LOG_DEFAULT_LEVEL >= 4  // ESP_LOG_DEBUG
  configASSERT(xTaskCreate(debug_loop, "debug_loop", STACK_SIZE, NULL, 1UL, NULL) == pdTRUE);
#endif

  // Initialize status LED
  status_start();

  // Initialize LED strip
  configASSERT(xTaskCreate(led_loop, "led_loop", STACK_SIZE, NULL, 2UL, NULL) == pdTRUE);

  // Initialize display
  display_start();

  // Initialize string display and show "Hi"
  strings_start();

  // Initialize all other components
  celebrate_start();
  wifi_start();
  sntp_start();

  configASSERT(xTaskCreate(light_loop, "light_loop", STACK_SIZE, NULL, 1UL, NULL) == pdTRUE);
  configASSERT(xTaskCreate(temp_loop, "temp_loop", STACK_SIZE, NULL, 1UL, NULL) == pdTRUE);
  configASSERT(xTaskCreate(clock_loop, "clock_loop", STACK_SIZE, NULL, 1UL, NULL) == pdTRUE);
  configASSERT(xTaskCreate(touch_loop, "touch_loop", STACK_SIZE, NULL, 1UL, NULL) == pdTRUE);

  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_TOUCH_TRIPLETAP, handle_touch, NULL, NULL));
}
