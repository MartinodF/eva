#include "eva_sntp.h"

static int64_t sntp_last_sync = 0;
static esp_timer_handle_t sntp_expired_timer;
static EventGroupHandle_t sntp_event_group;

static const char* TAG = "sntp";

void sntp_start(void) {
  ESP_LOGI(TAG, "sntp_start");

  // Set timezone
  setenv("TZ", EVA_TIMEZONE, 1);
  tzset();

  sntp_event_group = xEventGroupCreate();

  const esp_timer_create_args_t timer_args = {.callback = &sntp_timer_cb, .name = "sntp-expiry"};
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &sntp_expired_timer));

  // Enable SNTP time sync
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, EVA_TIMESERVER);
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
  sntp_set_sync_interval(EVA_SNTP_INTERVAL);
  sntp_set_time_sync_notification_cb(sntp_sync_cb);
  sntp_init();

  ESP_LOGI(TAG, "sntp_start done");
}

void sntp_sync_cb(struct timeval* tv) {
  ESP_LOGI(TAG, "sntp_sync");
  sntp_last_sync = esp_timer_get_time();
  xEventGroupSetBits(sntp_event_group, EVA_SNTP_HEALTHY_BIT);

  // Clear any running timer, we don't care about errors
  esp_timer_stop(sntp_expired_timer);
  ESP_ERROR_CHECK(esp_timer_start_once(sntp_expired_timer, EVA_SNTP_INTERVAL * 2 * 1000));
}

void sntp_timer_cb(void* param) {
  ESP_LOGW(TAG, "sntp_timer expired, time might be drifting");
  xEventGroupClearBits(sntp_event_group, EVA_SNTP_HEALTHY_BIT);
}

bool sntp_healthy() { return (xEventGroupGetBits(sntp_event_group) & EVA_SNTP_HEALTHY_BIT) == EVA_SNTP_HEALTHY_BIT; }
