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

static const clockleds names[] = {
    [Fede] = {10, 21, 32, 43}, [Mart] = {5, 6, 7, 8},   [June] = {38, 49, 60, 71},
    [Greg] = {9, 20, 31, 42},  [FM] = {36, 37, -1, -1},
};

static const char* TAG = "clock";

void or_bit(frame buffer, int n) {
  int word = n / EVA_DISPLAY_WORD_SIZE;
  int bit = (EVA_DISPLAY_WORD_SIZE - 1) - (n % EVA_DISPLAY_WORD_SIZE);

  buffer[word] |= (1 << bit);
}

void or_name(frame buffer, const clockleds name) {
  for (int i = 0; i < EVA_CLOCK_LEDS_SIZE; i++) {
    if (name[i] < 0) {
      continue;
    }

    or_bit(buffer, name[i]);
  }
}

void set_names(frame buffer, struct tm* time) {
  if (time->tm_mon == 8 && time->tm_mday == 3) {
    or_name(buffer, names[Fede]);
  }

  if (time->tm_mon == 1 && time->tm_mday == 26) {
    or_name(buffer, names[Mart]);
  }

  if (time->tm_mon == 3 && time->tm_mday == 1) {
    or_name(buffer, names[June]);
  }

  if (time->tm_mon == 0 && time->tm_mday == 15) {
    or_name(buffer, names[Greg]);
  }

  if (time->tm_mon == 8 && time->tm_mday == 15) {
    or_name(buffer, names[FM]);
  }

  if (EVA_CLOCK_DEBUG) {
    int all[5] = {Fede, Mart, June, Greg, FM};
    int name = all[time->tm_min % 5];
    or_name(buffer, names[name]);
  }
}

void or_time(frame buffer, const clockmask* time) {
  for (int i = 0; i < EVA_CLOCK_MASK_SIZE; i++) {
    buffer[time->offset + i] |= time->words[i];
  }
}

void set_time(frame buffer, struct tm* time) {
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

  frame_event_t event;

  time_t rawtime;
  struct tm* timeinfo;

  for (;;) {
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    memset(event.mem, 0, sizeof(event.mem));
    set_time(event.mem, timeinfo);
    event.layer = LayerClock;
    event.skip_emit = true;

    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));

    memset(event.mem, 0, sizeof(frame));
    set_names(event.mem, timeinfo);
    event.layer = LayerEvents;
    event.skip_emit = false;

    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_FRAME_EMIT, &event, sizeof(event), portMAX_DELAY));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  ESP_LOGW(TAG, "clock_loop exited");
}
