#include "eva_strings.h"

#include "font.h"

static const char *TAG = "strings";

static void set_char(frame buffer, float position, char c) {
  int match = 0;

  for (size_t i = 0; font[i].letter != 0; i++) {
    if (font[i].letter == c) {
      match = i;
      break;
    }
  }

  ESP_LOGD(TAG, "printing character %c", font[match].letter);

  int offsetY = 2;
  int offsetX = roundf(position * (EVA_STRINGS_WIDTH + 1) + 1);

  for (size_t y = 0; y < EVA_STRINGS_HEIGHT; y++) {
    for (size_t x = 0; x < EVA_STRINGS_WIDTH; x++) {
      if (font[match].code[y][x] == '#') {
        int n = EVA_DISPLAY_WIDTH * (y + offsetY) + x + offsetX;
        int word = n / EVA_DISPLAY_WORD_SIZE;
        int bit = (EVA_DISPLAY_WORD_SIZE - 1) - (n % EVA_DISPLAY_WORD_SIZE);

        buffer[word] |= (1 << bit);
      }
    }
  }
}

static void render_string(char string[2], frame buffer) {
  memset(buffer, 0, sizeof(frame));

  if (string[1] == '\0') {
    set_char(buffer, 0.5f, string[0]);
  } else {
    set_char(buffer, 0.0f, string[0]);
    set_char(buffer, 1.0f, string[1]);
  }
}

void strings_animate(char *str, uint8_t times, int layer) {
  int length = strlen(str);
  char *padded = calloc(length + 4, sizeof(char));

  padded[0] = ' ';
  padded[1] = ' ';
  memcpy(padded + 2, str, length);
  padded[length + 2] = ' ';
  padded[length + 3] = '\0';

  ESP_LOGD(TAG, "animating '%s' (%d chars + padding) %d times on layer %d", padded, length, times, layer);

  frame_event_t event = {.layer = layer};

  while (times-- > 0) {
    for (size_t index = 0; padded[index + 1]; index++) {
      render_string(padded + index, event.mem);
      ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));

      vTaskDelay(pdMS_TO_TICKS(EVA_STRINGS_FRAME_TIME));
    }
  }
}

void strings_set(char value[2], int layer) {
  frame_event_t event = {.layer = layer};
  render_string(value, event.mem);

  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));
}

void strings_set_int(int value, int layer) {
  int units = value % 10;
  int tens = (value / 10) % 10;

  char string[2] = {tens + '0', units + '0'};

  if (tens == 0) {
    string[0] = string[1];
    string[1] = '\0';
  }

  strings_set(string, layer);
}

static void light_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  int value = *((int *)event_data);
  strings_set_int(value, LayerLight);
}

static void temp_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  int value = roundf(*((float *)event_data));
  strings_set_int(value, LayerTemp);
}

static void status_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  strings_set(event_data, LayerStatus);
}

void strings_start() {
  // Setup a few initial frames
  strings_set((char *)"HI", LayerBooting);
  strings_set((char *)"WI", LayerStatus);

  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_LIGHT_UPDATE, light_update, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_TEMP_UPDATE, temp_update, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_STATUS_HEALTHY, status_update, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_STATUS_UNHEALTHY, status_update, NULL, NULL));
}
