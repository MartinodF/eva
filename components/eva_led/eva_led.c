#include "eva_led.h"

static int light;
static clockbits bits[EVA_CLOCK_LAYERS];
static uint8_t pixels[EVA_CLOCK_CHARS * EVA_LED_COLORS];  // GRBW
static float fading[EVA_CLOCK_CHARS * 2];                 // W + GRB brightness

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

  gpio_set_direction(EVA_ONBOARD_GPIO_NUM, GPIO_MODE_OUTPUT);

  rmt_leds_handle_t leds = NULL;
  ESP_ERROR_CHECK(rmt_new_leds(EVA_LED_GPIO_NUM, EVA_CLOCK_CHARS, pixels, &leds));

  memset(pixels, 0, sizeof(pixels));

  uint32_t r;
  uint32_t g;
  uint32_t b;
  uint16_t hue = 0;
  float scale;

  for (;;) {
    gpio_set_level(EVA_ONBOARD_GPIO_NUM, 1);

    clock_get(bits);
    light_get(&light);

    hs2rgb(hue, 100, &r, &g, &b);
    scale = 0.08f + 0.92f * light / 4095;

    if (EVA_LED_DEBUG_FRAMES) {
      print_chars(bits[0]);
      print_chars(bits[1]);
    }

    for (int i = 0; i < EVA_CLOCK_CHARS; i++) {
      bool base = has_bit(bits[0], i);
      bool extra = has_bit(bits[1], i);

      // Rows alternate in a zig-zag pattern
      int row = i / EVA_CLOCK_WIDTH;
      int column = (row % 2 == 0) ? (i % EVA_CLOCK_WIDTH) : ((EVA_CLOCK_WIDTH - 1) - (i % EVA_CLOCK_WIDTH));

      int led = row * EVA_CLOCK_WIDTH + column;

      // Smooth out changes in brightness
      fading[led * 2 + 0] = 0.9f * fading[led * 2 + 0] + 0.1f * (base ? scale : 0);
      fading[led * 2 + 1] = 0.9f * fading[led * 2 + 1] + 0.1f * (extra ? scale : 0);

      pixels[led * EVA_LED_COLORS + 0] = g * fading[led * 2 + 1];     // G
      pixels[led * EVA_LED_COLORS + 1] = r * fading[led * 2 + 1];     // R
      pixels[led * EVA_LED_COLORS + 2] = b * fading[led * 2 + 1];     // B
      pixels[led * EVA_LED_COLORS + 3] = 0xff * fading[led * 2 + 0];  // W
    }

    ESP_ERROR_CHECK(rmt_leds_send(leds));

    hue += 1;

    gpio_set_level(EVA_ONBOARD_GPIO_NUM, 0);
    vTaskDelay(pdMS_TO_TICKS(EVA_LED_INTERVAL));
  }

  ESP_ERROR_CHECK(rmt_leds_del(leds));

  ESP_LOGW(TAG, "led_loop exited");
}
