#include "eva_sntp.h"

static int64_t sntp_last_sync = 0;
static esp_event_handler_instance_t wifi_handle = NULL;
static esp_timer_handle_t sntp_expired_timer;

static const char* TAG = "sntp";

static void sntp_sync_cb(struct timeval* tv) {
  ESP_LOGI(TAG, "sntp_sync");
  sntp_last_sync = esp_timer_get_time();
  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_SNTP_HEALTHY, NULL, 0, portMAX_DELAY));

  // Clear any running timer, we don't care about errors
  esp_timer_stop(sntp_expired_timer);
  ESP_ERROR_CHECK(esp_timer_start_once(sntp_expired_timer, EVA_SNTP_INTERVAL * 2 * 1000));
}

static void sntp_timer_cb(void* param) {
  ESP_LOGW(TAG, "sntp_timer expired, time might be drifting");
  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_SNTP_UNHEALTHY, NULL, 0, portMAX_DELAY));
}

static void sntp_wifi_cb() {
  ESP_LOGI(TAG, "sntp_wifi");

  vTaskDelay(pdMS_TO_TICKS(10));

  esp_event_handler_instance_unregister(EVA_EVENT, EVA_WIFI_HEALTHY, wifi_handle);

  const esp_timer_create_args_t timer_args = {.callback = &sntp_timer_cb, .name = "sntp-expiry"};
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &sntp_expired_timer));

  // Enable SNTP time sync
  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, EVA_TIMESERVER);
  esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
  esp_sntp_set_sync_interval(EVA_SNTP_INTERVAL);
  esp_sntp_set_time_sync_notification_cb(sntp_sync_cb);
  esp_sntp_init();
}

void sntp_start(void) {
  ESP_LOGI(TAG, "sntp_start");

  // Set timezone
  setenv("TZ", EVA_TIMEZONE, 1);
  tzset();

  // Wait for a Wi-Fi connection before initializing
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_WIFI_HEALTHY, sntp_wifi_cb, NULL, &wifi_handle));

  ESP_LOGI(TAG, "sntp_start done");
}
