#include "eva_led.h"

static const char* TAG = "led";

void led_loop(void* param) {
  ESP_LOGI(TAG, "led_loop starting");
  clockbits bits[EVA_CLOCK_LAYERS];

  for (;;) {
    clock_get(bits);
    print_chars(bits[0]);
    print_chars(bits[1]);

    vTaskDelay(pdMS_TO_TICKS(30000));
  }

  ESP_LOGI(TAG, "led_loop exited");
}
