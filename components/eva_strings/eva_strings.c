#include "eva_strings.h"

#include "font.h"

static const char *TAG = "strings";

static void set_char(frame buffer, int position, char c) {
  int match = 0;

  for (int i = 0; font[i].letter != 0; i++) {
    if (font[i].letter == c) {
      match = i;
      break;
    }
  }

  ESP_LOGD(TAG, "printing character %c", font[match].letter);

  int offsetY = 2;
  int offsetX = 1 + position * (EVA_STRINGS_WIDTH + 1);

  for (int y = 0; y < EVA_STRINGS_HEIGHT; y++) {
    for (int x = 0; x < EVA_STRINGS_WIDTH; x++) {
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

  set_char(buffer, 0, string[0]);
  set_char(buffer, 1, string[1]);
}

static void render_number(int value, frame buffer) {
  int units = value % 10;
  int tens = (value / 10) % 10;

  char string[2] = {tens + '0', units + '0'};

  render_string(string, buffer);
}

static void light_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  int value = *((int *)event_data);

  frame_event_t event = {.layer = LayerLight};
  render_number(value, event.mem);

  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));
}

static void temp_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  int value = roundf(*((float *)event_data));

  frame_event_t event = {.layer = LayerTemp};
  render_number(value, event.mem);

  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));
}

static void status_update(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  frame_event_t event = {.layer = LayerStatus};
  render_string(event_data, event.mem);

  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));
}

void strings_start() {
  // Setup a few initial frames
  frame_event_t event = {.layer = LayerBooting};

  char string[2] = {'H', 'I'};
  render_string(string, event.mem);
  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));

  event.layer = LayerStatus;
  string[0] = 'W';
  string[1] = 'I';
  render_string(string, event.mem);
  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_LIGHT_UPDATE, light_update, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_TEMP_UPDATE, temp_update, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_STATUS_HEALTHY, status_update, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(EVA_EVENT, EVA_STATUS_UNHEALTHY, status_update, NULL, NULL));
}
