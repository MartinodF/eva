#include "eva_status.h"

static uint8_t pixel[EVA_LED_COLORS];  // GRBW

static const char* TAG = "status";

void status_loop(void*) {
  ESP_LOGI(TAG, "status_loop starting");

  rmt_leds_handle_t led = NULL;
  ESP_ERROR_CHECK(rmt_new_leds(EVA_STATUS_GPIO, 1, pixel, &led));

  bool sntp, wifi, ok, partial;

  for (;;) {
    sntp = sntp_healthy();
    wifi = wifi_available();

    ESP_LOGD(TAG, "SNTP: %s", sntp ? "healthy" : "unhealthy");
    ESP_LOGD(TAG, "WiFi: %s", wifi ? "healthy" : "unhealthy");

    ok = sntp && wifi;
    partial = sntp || wifi;

    pixel[0] = (ok ? 0xff : partial ? 0xff : 0x00) * EVA_LED_BRIGHTNESS;  // G
    pixel[1] = (ok ? 0x00 : partial ? 0xff : 0xff) * EVA_LED_BRIGHTNESS;  // R
    pixel[2] = (ok ? 0x00 : partial ? 0x00 : 0x00) * EVA_LED_BRIGHTNESS;  // B
    pixel[3] = 0 * EVA_LED_BRIGHTNESS;                                    // W

    ESP_ERROR_CHECK(rmt_leds_send(led));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  ESP_LOGW(TAG, "status_loop exited");
}
