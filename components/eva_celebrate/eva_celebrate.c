#include "eva_celebrate.h"

static const leds names[] = {
    [Fede] = {10, 21, 32, 43}, [Mart] = {5, 6, 7, 8},   [June] = {38, 49, 60, 71},
    [Greg] = {9, 20, 31, 42},  [FM] = {36, 37, -1, -1},
};

static celebration_t celebrations[] = {
    {.id = "new-year", .month = 0, .day = 1, .name = -1, .greeting = "HAPPY NEW YEAR!"},
    {.id = "greg", .month = 0, .day = 15, .name = Greg, .greeting = "HAPPY BIRTHDAY GREG!"},
    {.id = "mart", .month = 1, .day = 26, .name = Mart, .greeting = "HAPPY BIRTHDAY MART!"},
    {.id = "june", .month = 3, .day = 1, .name = June, .greeting = "HAPPY MEOWDAY!"},
    {.id = "fede", .month = 8, .day = 3, .name = Fede, .greeting = "HAPPY BIRTHDAY FEDE!"},
    {.id = "anniversary", .month = 8, .day = 15, .name = FM, .greeting = "HAPPY ANNIVERSARY!"},
};

static esp_event_handler_instance_t sntp_handle = NULL;

static const char* TAG = "celebrate";

static void or_bit(frame buffer, int n) {
  int word = n / EVA_DISPLAY_WORD_SIZE;
  int bit = (EVA_DISPLAY_WORD_SIZE - 1) - (n % EVA_DISPLAY_WORD_SIZE);

  buffer[word] |= (1 << bit);
}

static void or_name(frame buffer, const leds name) {
  for (int i = 0; i < EVA_CELEBRATE_NAME_LENGTH; i++) {
    if (name[i] < 0) {
      continue;
    }

    or_bit(buffer, name[i]);
  }
}

static void set_names(frame buffer, struct tm* time) {
  int length = sizeof(celebrations) / sizeof(celebrations[0]);

  for (int i = 0; i < length; i++) {
    if (time->tm_mon == celebrations[i].month && time->tm_mday == celebrations[i].day) {
      if (celebrations[i].name < 0) {
        continue;
      }

      or_name(buffer, names[celebrations[i].name]);
    }
  }
}

static void celebrate_loop(void* unused) {
  ESP_LOGI(TAG, "celebrate_loop starting");

  int length = sizeof(celebrations) / sizeof(celebrations[0]);
  frame_event_t event = {.layer = LayerEvents};

  time_t rawtime;
  struct tm* timeinfo;

  for (;;) {
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    for (int i = 0; i < length; i++) {
      if (celebrations[i].trigger < rawtime) {
        ESP_LOGI(TAG, "%s needs an updated trigger", celebrations[i].id);

        struct tm trigger = {
            .tm_year = timeinfo->tm_year,
            .tm_mon = celebrations[i].month,
            .tm_mday = celebrations[i].day,
        };

        celebrations[i].trigger = mktime(&trigger);

        if (celebrations[i].trigger < rawtime) {
          ESP_LOGI(TAG, "%s happens next year", celebrations[i].id);

          trigger.tm_year++;
          celebrations[i].trigger = mktime(&trigger);
        }

        ESP_LOGI(TAG, "%s triggering at %llu", celebrations[i].id, celebrations[i].trigger);
      }

      // Handle active celebrations
      double diff = difftime(celebrations[i].trigger, rawtime);
      if (diff < 61.0f) {
        if (diff > 0.0f) {
          ESP_LOGI(TAG, "%s in countdown: %d", celebrations[i].id, (int)floor(diff));
          strings_set_int(floor(diff), LayerCountdown);
        } else {
          ESP_LOGI(TAG, "%s animating", celebrations[i].id);
          strings_animate((char*)celebrations[i].greeting, 3, LayerCountdown);
        }
      }
    }

    memset(event.mem, 0, sizeof(frame));
    set_names(event.mem, timeinfo);

    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  ESP_LOGW(TAG, "celebrate_loop exited");
}

static void celebrate_sntp_cb() {
  ESP_LOGI(TAG, "celebrate_sntp");

  vTaskDelay(pdMS_TO_TICKS(10));

  esp_event_handler_instance_unregister(EVA_EVENT, EVA_SNTP_HEALTHY, sntp_handle);

  configASSERT(xTaskCreate(celebrate_loop, "celebrate_loop", STACK_SIZE, NULL, 1UL, NULL) == pdTRUE);
}

void celebrate_start(void) {
  ESP_LOGI(TAG, "celebrate_start");

  // Wait for an SNTP sync before initializing
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(EVA_EVENT, EVA_SNTP_HEALTHY, celebrate_sntp_cb, NULL, &sntp_handle));

  ESP_LOGI(TAG, "celebrate_start done");
}
