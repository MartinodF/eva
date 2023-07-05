#include "eva_display.h"

#if CONFIG_LOG_DEFAULT_LEVEL >= 4  // ESP_LOG_DEBUG
// clang-format off
static const char characters[EVA_DISPLAY_PIXELS] = {
  'I','T','L','I','S','M','A','R', 'T','G','F',  // 0 - 1
  'D','A','C','Q','U', 'A','R','T','E','R','E',  // 1 - 2
  'T','W', 'E','N','T','Y','F','I','V','E', 'D', // 2 - 3 - 4
  'H','A','L','F','M','J','T', 'E','N','G','E',  // 4 - 5
  'P','A','S','T', 'O','U','R','N','I','N','E',  // 5 - 6
  'E', 'L','E','V','E','N','E','I','G', 'H','T', // 6 - 7 - 8
  'S','I','X','O','N','E', 'T','H','R','E','E',  // 8 - 9
  'F','O','U', 'R','F','I','V','E','T','W','O',  // 9 - 10
  'S','E','V','E','N','T','W','E', 'L','V','E',  // 11 - 12
  'T','E','N','S','E', 'O','C','L','O','C','K',  // 12 - 13
};
// clang-format on
#endif

static esp_timer_handle_t override_timer;
static int current_layer = LayerClock;
static int override_layer = LayerBooting;
static buffer fb;
static bool healthy = false;

static const char* TAG = "display";

static bool has_bit(frame a, int n) {
  int word = n / EVA_DISPLAY_WORD_SIZE;
  int bit = (EVA_DISPLAY_WORD_SIZE - 1) - (n % EVA_DISPLAY_WORD_SIZE);

  return (a[word] >> bit) & 1;
}

static void emit_refresh() {
  pixels visible;

  int layer = (override_layer != LayersCount) ? override_layer : current_layer;

  for (size_t i = 0; i < EVA_DISPLAY_PIXELS; i++) {
    switch (layer) {
      case LayerClock:
        visible[i] = (has_bit(fb[LayerClock], i) ? White : Off) | (has_bit(fb[LayerEvents], i) ? Rainbow : Off);
        break;

      default:
        visible[i] = has_bit(fb[layer], i) ? Rainbow : Off;
        break;
    }
  }

  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_DISPLAY_REFRESH, &visible, sizeof(visible), portMAX_DELAY));

  ESP_LOGD(TAG, "emitted display refresh");
}

#if CONFIG_LOG_DEFAULT_LEVEL >= 4  // ESP_LOG_DEBUG
static void print_buffer() {
  for (size_t layer = LayerBooting; layer < LayersCount; layer++) {
    printf("# Layer %d\n", layer);

    for (size_t i = 0; i < EVA_DISPLAY_PIXELS; i++) {
      bool enabled = has_bit(fb[layer], i);

      printf("%c", enabled ? characters[i] : '-');

      if ((i + 1) % EVA_DISPLAY_WIDTH == 0) {
        printf("\n");
      }
    }
  }
}
#endif

static void handle_frame(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  frame_event_t* event = (frame_event_t*)event_data;

  ESP_LOGD(TAG, "received frame for layer %d", event->layer);

  // Avoid re-render if nothing has changed
  if (memcmp(&fb[event->layer], event->mem, sizeof(event->mem)) == 0) {
    ESP_LOGD(TAG, "skipping unchanged frame");
    return;
  }

  memcpy(&fb[event->layer], event->mem, sizeof(event->mem));

#if CONFIG_LOG_DEFAULT_LEVEL >= 4  // ESP_LOG_DEBUG
  print_buffer();
#endif

  // Countdown layer automatically overrides others for 2 seconds
  if (event->layer == LayerCountdown) {
    override_layer = LayerCountdown;

    if (esp_timer_is_active(override_timer)) {
      ESP_ERROR_CHECK(esp_timer_stop(override_timer));
    }

    ESP_ERROR_CHECK(esp_timer_start_once(override_timer, 2000 * 1000));
  }

  if (!event->skip_emit) {
    emit_refresh();
  }
}

static void display_layer(int layer) {
  current_layer = layer;

  ESP_LOGI(TAG, "switching to layer %d", layer);

  emit_refresh();
}

static void override_timer_cb(void* unused) {
  ESP_LOGD(TAG, "override timer elapsed");

  override_layer = LayersCount;
  display_layer(current_layer);
}

static void handle_boot(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (healthy) {
    override_timer_cb(0);
  } else {
    override_layer = LayerStatus;
    emit_refresh();
  }
}

static void handle_status(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  healthy = event_id == EVA_STATUS_HEALTHY;

  if (healthy) {
    override_timer_cb(0);
  } else if (override_layer != LayerBooting) {
    override_layer = LayerStatus;
    emit_refresh();
  }
}

static void handle_touch(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_id == EVA_TOUCH_TAP) {
    int next = (current_layer + 1) % LayersVisible;
    display_layer(next);
  } else if (event_id == EVA_TOUCH_HOLD) {
    display_layer(LayerClock);
  }
}

void display_start(void) {
  ESP_LOGI(TAG, "display_start");

  const esp_timer_create_args_t timer_args = {.callback = &override_timer_cb, .name = "override-timer"};
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &override_timer));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_BOOTED, handle_boot, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_FRAME_EMIT, handle_frame, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_STATUS_HEALTHY, handle_status, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_STATUS_UNHEALTHY, handle_status, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_TOUCH_TAP, handle_touch, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_TOUCH_HOLD, handle_touch, NULL, NULL));

  ESP_LOGI(TAG, "display_start done");
}
