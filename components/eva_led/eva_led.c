#include "eva_led.h"

static uint32_t hue = 0;
static uint32_t light_time = 0;
static int light_prev = 0;
static int light_end = 0;

SemaphoreHandle_t barrier = NULL;
static fading fade[EVA_DISPLAY_PIXELS];
static bool needs_refreshing = true;

static const char *TAG = "led";

static void hs2rgb(uint32_t h, uint32_t s, uint32_t *r, uint32_t *g, uint32_t *b) {
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

static float_t interpolate(float_t x, uint32_t from, uint32_t to) {
  if (x >= 1.0f) {
    return to;
  }

  if (x <= 0.0f) {
    return from;
  }

  return from * (1.0f - x) + (to * x);
}

static float_t tween(uint32_t t, uint32_t t0, uint32_t duration) {
  if (t < t0) {
    return 0.0f;
  }

  if (t > t0 + duration) {
    return 1.0f;
  }

  return ((float_t)t - t0) / duration;
}

static void handle_frame_emit(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "received frame...");

  uint32_t time = xTaskGetTickCount();

  pixels *display = (pixels *)event_data;

  if (xSemaphoreTake(barrier, pdMS_TO_TICKS(EVA_LED_INTERVAL)) != pdTRUE) {
    ESP_LOGW(TAG, "failed acquiring lock to handle frame");
    return;
  }

  for (size_t i = 0; i < EVA_DISPLAY_PIXELS; i++) {
    if ((fade[i].endW == 0 && ((*display)[i] & White) == 0) ||
        (fade[i].endW == 0xFF && ((*display)[i] & White) == White)) {
      if ((fade[i].endR == 0 && ((*display)[i] & Rainbow) == 0) ||
          (fade[i].endR == 0xFF && ((*display)[i] & Rainbow) == Rainbow)) {
        // No change
        continue;
      }
    }

    needs_refreshing = true;

    float_t time_t = tween(time, fade[i].start, EVA_LED_FADE_DURATION);

    fade[i].start = time;
    fade[i].prevW = interpolate(time_t, fade[i].prevW, fade[i].endW);
    fade[i].prevR = interpolate(time_t, fade[i].prevR, fade[i].endR);
    fade[i].endW = ((*display)[i] & White) == White ? 0xFF : 0;
    fade[i].endR = ((*display)[i] & Rainbow) == Rainbow ? 0xFF : 0;
  }

  xSemaphoreGive(barrier);
}

static void handle_light_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "received light update...");

  uint32_t time = xTaskGetTickCount();

  if (xSemaphoreTake(barrier, pdMS_TO_TICKS(EVA_LED_INTERVAL)) != pdTRUE) {
    ESP_LOGW(TAG, "failed acquiring lock to handle light");
    return;
  }

  needs_refreshing = true;

  float_t time_t = tween(time, light_time, EVA_LED_FADE_DURATION);

  light_time = time;
  light_prev = interpolate(time_t, light_prev, light_end);
  light_end = *((int *)event_data);

  xSemaphoreGive(barrier);
}

void led_loop(void *unused) {
  ESP_LOGI(TAG, "led_loop starting");

  gpio_set_direction(EVA_ONBOARD_GPIO_NUM, GPIO_MODE_OUTPUT);

  led_strip_config_t strip_config = {
      .strip_gpio_num = EVA_LED_GPIO_NUM,
      .max_leds = EVA_DISPLAY_PIXELS,
      .led_pixel_format = LED_PIXEL_FORMAT_GRBW,
      .led_model = LED_MODEL_SK6812,
  };

  led_strip_spi_config_t spi_config = {
      .clk_src = RMT_CLK_SRC_XTAL,
      .spi_bus = SPI2_HOST,
      .flags.with_dma = true,
  };

  led_strip_handle_t leds;
  ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &leds));

  memset(fade, 0, sizeof(fade));

  barrier = xSemaphoreCreateBinary();
  xSemaphoreGive(barrier);

  uint32_t r, g, b, w;

  esp_event_handler_instance_t display_handle = NULL;
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(EVA_EVENT, EVA_DISPLAY_REFRESH, handle_frame_emit, NULL, &display_handle));

  esp_event_handler_instance_t light_handle = NULL;
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(EVA_EVENT, EVA_LIGHT_UPDATE, handle_light_update, NULL, &light_handle));

  uint32_t time = xTaskGetTickCount();

  for (;;) {
    if (xSemaphoreTake(barrier, pdMS_TO_TICKS(EVA_LED_INTERVAL)) != pdTRUE) {
      ESP_LOGW(TAG, "failed acquiring lock to update LEDs");
      continue;
    }

    // If no tweens are in progress, avoid refreshing
    if (!needs_refreshing) {
      xSemaphoreGive(barrier);

      if (xTaskDelayUntil(&time, pdMS_TO_TICKS(EVA_LED_INTERVAL)) == pdFALSE) {
        ESP_LOGW(TAG, "led_loop saturated");
      }

      continue;
    }

    ESP_LOGD(TAG, "refreshing leds");
    ESP_ERROR_CHECK(gpio_set_level(EVA_ONBOARD_GPIO_NUM, 1));

    // LED brightness varies from 1% to 55% depending on measured light
    float_t light_t = tween(time, light_time, EVA_LED_FADE_DURATION);
    float_t scale = 0.01f + 0.54f * interpolate(light_t, light_prev, light_end) / 100;

    // If the light level tween is done, no need to refresh for it
    bool last_refresh = time >= light_time + EVA_LED_FADE_DURATION;

    hue = time / pdMS_TO_TICKS(EVA_LED_INTERVAL);

    for (size_t i = 0; i < EVA_DISPLAY_PIXELS; i++) {
      // Rows alternate in a zig-zag pattern
      int row = i / EVA_DISPLAY_WIDTH;
      int column = (row % 2 == 0) ? (i % EVA_DISPLAY_WIDTH) : ((EVA_DISPLAY_WIDTH - 1) - (i % EVA_DISPLAY_WIDTH));

      int led = row * EVA_DISPLAY_WIDTH + column;

      // Fast path when we're not tweening
      if (time >= fade[i].start + EVA_LED_FADE_DURATION) {
        if (fade[i].endR > 0) {
          // Colors require continuous refreshing
          last_refresh = false;

          int ledhue = hue + 5 * (row + (i % EVA_DISPLAY_WIDTH));
          hs2rgb(ledhue, 100, &r, &g, &b);

          r = scale * r * fade[i].endR / 0xFF;
          g = scale * g * fade[i].endR / 0xFF;
          b = scale * b * fade[i].endR / 0xFF;
        } else {
          r = 0;
          g = 0;
          b = 0;
        }

        w = scale * fade[i].endW;
      } else {
        // Some leds are still tweening
        last_refresh = false;

        float_t time_t = tween(time, fade[i].start, EVA_LED_FADE_DURATION);

        if ((fade[i].prevR > 0) || (fade[i].endR > 0)) {
          int ledhue = hue + 5 * (row + (i % EVA_DISPLAY_WIDTH));
          hs2rgb(ledhue, 100, &r, &g, &b);

          int value = interpolate(time_t, fade[i].prevR, fade[i].endR);

          r = scale * r * value / 0xFF;
          g = scale * g * value / 0xFF;
          b = scale * b * value / 0xFF;
        } else {
          r = 0;
          g = 0;
          b = 0;
        }

        w = scale * interpolate(time_t, fade[i].prevW, fade[i].endW);
      }

      ESP_ERROR_CHECK(led_strip_set_pixel_rgbw(leds, led, r, g, b, w));
    }

    needs_refreshing = !last_refresh;

    xSemaphoreGive(barrier);

    ESP_ERROR_CHECK(led_strip_refresh(leds));
    ESP_ERROR_CHECK(gpio_set_level(EVA_ONBOARD_GPIO_NUM, 0));

    if (xTaskDelayUntil(&time, pdMS_TO_TICKS(EVA_LED_INTERVAL)) == pdFALSE) {
      ESP_LOGW(TAG, "led_loop saturated");
    }
  }

  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(EVA_EVENT, EVA_DISPLAY_REFRESH, display_handle));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(EVA_EVENT, EVA_LIGHT_UPDATE, light_handle));

  ESP_ERROR_CHECK(led_strip_del(leds));

  ESP_LOGW(TAG, "light_loop exited");
}
