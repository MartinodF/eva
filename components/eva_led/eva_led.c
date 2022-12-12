#include "eva_led.h"

static uint32_t hue = 0;
static uint32_t light_time = 0;
static int light_prev = 0;
static int light_end = 0;

SemaphoreHandle_t fade_barrier = NULL;
static fading fade[EVA_DISPLAY_PIXELS];

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

static void handle_frame_emit(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "received frame...");

  uint32_t time = xTaskGetTickCount();

  pixels *display = (pixels *)event_data;

  if (xSemaphoreTake(fade_barrier, pdMS_TO_TICKS(EVA_LED_INTERVAL)) != pdTRUE) {
    ESP_LOGW(TAG, "failed acquiring lock to handle frame");
    return;
  }

  for (int i = 0; i < EVA_DISPLAY_PIXELS; i++) {
    if (fade[i].end != (*display)[i]) {
      fade[i].prev = fade[i].end;
      fade[i].start = time;
      fade[i].end = (*display)[i];
    }
  }

  xSemaphoreGive(fade_barrier);
}

static void handle_light_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "received light update...");

  uint32_t time = xTaskGetTickCount();

  light_time = time;
  light_prev = light_end;
  light_end = *((int *)event_data);
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

void led_loop(void *unused) {
  ESP_LOGI(TAG, "led_loop starting");

  gpio_set_direction(EVA_ONBOARD_GPIO_NUM, GPIO_MODE_OUTPUT);

  rmt_leds_handle_t leds;
  uint8_t data[EVA_DISPLAY_PIXELS * EVA_LED_COLORS];  // GRBW

  ESP_ERROR_CHECK(rmt_new_leds(EVA_LED_GPIO_NUM, EVA_DISPLAY_PIXELS, data, &leds));
  memset(fade, 0, sizeof(fade));

  fade_barrier = xSemaphoreCreateBinary();
  xSemaphoreGive(fade_barrier);

  uint32_t r;
  uint32_t g;
  uint32_t b;

  esp_event_handler_instance_t display_handle = NULL;
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(EVA_EVENT, EVA_DISPLAY_REFRESH, handle_frame_emit, NULL, &display_handle));

  esp_event_handler_instance_t light_handle = NULL;
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(EVA_EVENT, EVA_LIGHT_UPDATE, handle_light_update, NULL, &light_handle));

  uint32_t time = xTaskGetTickCount();

  for (;;) {
    gpio_set_level(EVA_ONBOARD_GPIO_NUM, 1);

    float_t light_t = tween(time, light_time, EVA_LED_FADE_DURATION);
    float_t scale = 0.08f + 0.92f * interpolate(light_t, light_prev, light_end) / 4095;

    if (xSemaphoreTake(fade_barrier, pdMS_TO_TICKS(EVA_LED_INTERVAL)) != pdTRUE) {
      ESP_LOGW(TAG, "failed acquiring lock to update LEDs");
      continue;
    }

    for (int i = 0; i < EVA_DISPLAY_PIXELS; i++) {
      // Rows alternate in a zig-zag pattern
      int row = i / EVA_DISPLAY_WIDTH;
      int column = (row % 2 == 0) ? (i % EVA_DISPLAY_WIDTH) : ((EVA_DISPLAY_WIDTH - 1) - (i % EVA_DISPLAY_WIDTH));

      int led = row * EVA_DISPLAY_WIDTH + column;

      // Fast path when all tweens are done
      if (time >= fade[i].start + EVA_LED_FADE_DURATION) {
        if (fade[i].end & Rainbow) {
          int ledhue = hue + 5 * (row + (i % EVA_DISPLAY_WIDTH));
          hs2rgb(ledhue, 100, &r, &g, &b);

          data[led * EVA_LED_COLORS + 0] = scale * g;
          data[led * EVA_LED_COLORS + 1] = scale * r;
          data[led * EVA_LED_COLORS + 2] = scale * b;
        } else {
          memset(&data[led * EVA_LED_COLORS], 0, 3 * sizeof(uint8_t));
        }

        data[led * EVA_LED_COLORS + 3] = scale * ((fade[i].end & White) ? 0xff : 0);
      } else {
        float_t time_t = tween(time, fade[i].start, EVA_LED_FADE_DURATION);

        bool w_prev = (fade[i].prev & White) ? 1 : 0;
        bool c_prev = (fade[i].prev & Rainbow) ? 1 : 0;
        bool w_end = (fade[i].end & White) ? 1 : 0;
        bool c_end = (fade[i].end & Rainbow) ? 1 : 0;

        if ((fade[i].prev & Rainbow) || (fade[i].end & Rainbow)) {
          int ledhue = hue + 5 * (row + (i % EVA_DISPLAY_WIDTH));
          hs2rgb(ledhue, 100, &r, &g, &b);

          data[led * EVA_LED_COLORS + 0] = scale * g * ((1 - time_t) * c_prev + time_t * c_end);
          data[led * EVA_LED_COLORS + 1] = scale * r * ((1 - time_t) * c_prev + time_t * c_end);
          data[led * EVA_LED_COLORS + 2] = scale * b * ((1 - time_t) * c_prev + time_t * c_end);
        } else {
          memset(&data[led * EVA_LED_COLORS], 0, 3 * sizeof(uint8_t));
        }

        data[led * EVA_LED_COLORS + 3] = scale * 0xff * ((1 - time_t) * w_prev + time_t * w_end);
      }
    }

    xSemaphoreGive(fade_barrier);

    ESP_ERROR_CHECK(rmt_leds_send(leds));

    hue += 1;

    gpio_set_level(EVA_ONBOARD_GPIO_NUM, 0);

    if (xTaskDelayUntil(&time, EVA_LED_INTERVAL) == pdFALSE) {
      ESP_LOGW(TAG, "led_loop saturated");
    }
  }

  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(EVA_EVENT, EVA_DISPLAY_REFRESH, display_handle));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(EVA_EVENT, EVA_LIGHT_UPDATE, light_handle));

  ESP_ERROR_CHECK(rmt_leds_del(leds));

  ESP_LOGW(TAG, "light_loop exited");
}
