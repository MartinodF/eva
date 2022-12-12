#include "eva_status.h"

static uint8_t pixel[EVA_LED_COLORS];  // GRBW
static rmt_leds_handle_t led = NULL;
static bool status[StatusesCount] = {0};

static const char* TAG = "status";

static void handle_status(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  switch (event_id) {
    case EVA_SNTP_HEALTHY:
      status[SNTP] = true;
      break;
    case EVA_SNTP_UNHEALTHY:
      status[SNTP] = false;
      break;
    case EVA_WIFI_HEALTHY:
      status[WiFi] = true;
      break;
    case EVA_WIFI_UNHEALTHY:
      status[WiFi] = false;
      break;
  }

  bool ok = status[WiFi] && status[SNTP];
  bool partial = status[WiFi] || status[SNTP];

  ESP_LOGI(TAG, "WiFi: %s", status[WiFi] ? "healthy" : "unhealthy");
  ESP_LOGI(TAG, "SNTP: %s", status[SNTP] ? "healthy" : "unhealthy");

  pixel[0] = (ok ? 0xff : partial ? 0xff : 0x00) * EVA_STATUS_BRIGHTNESS;  // G
  pixel[1] = (ok ? 0x00 : partial ? 0xff : 0xff) * EVA_STATUS_BRIGHTNESS;  // R
  pixel[2] = (ok ? 0x00 : partial ? 0x00 : 0x00) * EVA_STATUS_BRIGHTNESS;  // B
  pixel[3] = 0 * EVA_STATUS_BRIGHTNESS;                                    // W

  ESP_ERROR_CHECK(rmt_leds_send(led));

  if (ok) {
    char reason[2] = "OK";
    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_STATUS_HEALTHY, &reason, sizeof(reason), portMAX_DELAY));
  } else {
    if (!status[SNTP]) {
      char reason[2] = "SY";
      ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_STATUS_UNHEALTHY, &reason, sizeof(reason), portMAX_DELAY));
    } else if (!status[WiFi]) {
      char reason[2] = "WI";
      ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_STATUS_UNHEALTHY, &reason, sizeof(reason), portMAX_DELAY));
    }
  }
}

void status_start() {
  ESP_LOGI(TAG, "status_start");

  ESP_ERROR_CHECK(rmt_new_leds(EVA_STATUS_GPIO, 1, pixel, &led));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_SNTP_HEALTHY, handle_status, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_SNTP_UNHEALTHY, handle_status, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_WIFI_HEALTHY, handle_status, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_WIFI_UNHEALTHY, handle_status, NULL, NULL));

  ESP_LOGI(TAG, "status_start done");
}
