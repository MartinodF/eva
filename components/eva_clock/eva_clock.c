#include "eva_clock.h"

static const clockmask words[] = {
    [ItIs] = {0, {0b11011000, 0b00000000}},
    [FiveMinutes] = {3, {0b00001111, 0b00000000}},
    [TenMinutes] = {4, {0b00000001, 0b11000000}},
    [AQuarter] = {1, {0b00001011, 0b11111000}},
    [TwentyMinutes] = {2, {0b00000011, 0b11110000}},
    [Half] = {4, {0b01111000, 0b00000000}},
    [To] = {5, {0b00000001, 0b10000000}},
    [Past] = {5, {0b00001111, 0b00000000}},
    [OClock] = {12, {0b00000000, 0b11111100}},
    [Twelve] = {11, {0b00000111, 0b11100000}},
    [One] = {8, {0b00000111, 0b00000000}},
    [Two] = {10, {0b00000111, 0b00000000}},
    [Three] = {9, {0b11111000, 0b00000000}},
    [Four] = {9, {0b00000111, 0b10000000}},
    [Five] = {10, {0b01111000, 0b00000000}},
    [Six] = {8, {0b00111000, 0b00000000}},
    [Seven] = {11, {0b11111000, 0b00000000}},
    [Eight] = {7, {0b00000111, 0b11000000}},
    [Nine] = {6, {0b00011110, 0b00000000}},
    [Ten] = {12, {0b00011100, 0b00000000}},
    [Eleven] = {6, {0b00000001, 0b11111000}},
};

static const char* TAG = "clock";

static void or_time(frame buffer, const clockmask* time) {
  for (int i = 0; i < EVA_CLOCK_MASK_SIZE; i++) {
    buffer[time->offset + i] |= time->words[i];
  }
}

static void set_time(frame buffer, struct tm* time) {
  or_time(buffer, &words[ItIs]);

  if (time->tm_min < 5) {
    or_time(buffer, &words[OClock]);
  } else if ((time->tm_min >= 5 && time->tm_min < 10) || (time->tm_min >= 55)) {
    or_time(buffer, &words[FiveMinutes]);
  } else if ((time->tm_min >= 10 && time->tm_min < 15) || (time->tm_min >= 50 && time->tm_min < 55)) {
    or_time(buffer, &words[TenMinutes]);
  } else if ((time->tm_min >= 15 && time->tm_min < 20) || (time->tm_min >= 45 && time->tm_min < 50)) {
    or_time(buffer, &words[AQuarter]);
  } else if ((time->tm_min >= 20 && time->tm_min < 25) || (time->tm_min >= 40 && time->tm_min < 45)) {
    or_time(buffer, &words[TwentyMinutes]);
  } else if ((time->tm_min >= 25 && time->tm_min < 30) || (time->tm_min >= 35 && time->tm_min < 40)) {
    or_time(buffer, &words[TwentyMinutes]);
    or_time(buffer, &words[FiveMinutes]);
  } else {
    or_time(buffer, &words[Half]);
  }

  int hour = time->tm_hour;

  if (time->tm_min >= 5 && time->tm_min < 35) {
    or_time(buffer, &words[Past]);
  } else if (time->tm_min >= 35) {
    or_time(buffer, &words[To]);
    hour++;
  }

  int hour_mask = Twelve + (hour % 12);
  or_time(buffer, &words[hour_mask]);
}

void clock_loop(void* unused) {
  ESP_LOGI(TAG, "clock_loop starting");

  frame_event_t event = {.layer = LayerClock};

  time_t rawtime;
  struct tm* timeinfo;

  for (;;) {
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    memset(event.mem, 0, sizeof(event.mem));
    set_time(event.mem, timeinfo);

    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  ESP_LOGW(TAG, "clock_loop exited");
}
