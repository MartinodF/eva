#include "eva_led.h"

static clockbits bits[EVA_CLOCK_LAYERS];
static uint8_t pixels[EVA_CLOCK_CHARS * EVA_LED_COLORS];  // GRBW

static const char *TAG = "led";

void hs2rgb(uint32_t h, uint32_t s, uint32_t *r, uint32_t *g, uint32_t *b) {
  h %= 360;
  uint32_t rgb_max = 0xff;
  uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

  uint32_t i = h / 60;
  uint32_t diff = h % 60;

  uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

  switch (i) {
    case 0:
      *r = rgb_max;
      *g = rgb_min + rgb_adj;
      *b = rgb_min;
      break;
    case 1:
      *r = rgb_max - rgb_adj;
      *g = rgb_max;
      *b = rgb_min;
      break;
    case 2:
      *r = rgb_min;
      *g = rgb_max;
      *b = rgb_min + rgb_adj;
      break;
    case 3:
      *r = rgb_min;
      *g = rgb_max - rgb_adj;
      *b = rgb_max;
      break;
    case 4:
      *r = rgb_min + rgb_adj;
      *g = rgb_min;
      *b = rgb_max;
      break;
    default:
      *r = rgb_max;
      *g = rgb_min;
      *b = rgb_max - rgb_adj;
      break;
  }
}

void led_loop(void *) {
  ESP_LOGI(TAG, "led_loop starting");

  rmt_leds_handle_t leds = NULL;
  ESP_ERROR_CHECK(rmt_new_leds(EVA_LED_GPIO_NUM, EVA_CLOCK_CHARS, pixels, &leds));

  memset(pixels, 0, sizeof(pixels));

  uint32_t r;
  uint32_t g;
  uint32_t b;
  uint16_t hue = 0;
  bool has_extra;

  for (;;) {
    clock_get(bits);

    if (EVA_LED_DEBUG_FRAMES) {
      print_chars(bits[0]);
      print_chars(bits[1]);
    }

    has_extra = false;

    for (int i = 0; i < EVA_CLOCK_CHARS; i++) {
      bool base = has_bit(bits[0], i);
      bool extra = has_bit(bits[1], i);

      if (extra) {
        has_extra = true;
        hs2rgb(hue, 100, &r, &g, &b);
      } else {
        r = 0;
        g = 0;
        b = 0;
      }

      // Rows alternate in a zig-zag pattern
      int row = i / EVA_CLOCK_WIDTH;
      int column = (row % 2 == 0) ? (i % EVA_CLOCK_WIDTH) : ((EVA_CLOCK_WIDTH - 1) - (i % EVA_CLOCK_WIDTH));

      int led = row * EVA_CLOCK_WIDTH + column;

      pixels[led * EVA_LED_COLORS + 0] = g * EVA_LED_BRIGHTNESS;                  // G
      pixels[led * EVA_LED_COLORS + 1] = r * EVA_LED_BRIGHTNESS;                  // R
      pixels[led * EVA_LED_COLORS + 2] = b * EVA_LED_BRIGHTNESS;                  // B
      pixels[led * EVA_LED_COLORS + 3] = (base ? 0xff : 0) * EVA_LED_BRIGHTNESS;  // W
    }

    ESP_ERROR_CHECK(rmt_leds_send(leds));

    if (has_extra) {
      hue += 1;
      vTaskDelay(pdMS_TO_TICKS(80));
    } else {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  ESP_ERROR_CHECK(rmt_leds_del(leds));

  ESP_LOGW(TAG, "led_loop exited");
}
