#include "eva_clock.h"

// clang-format off
static const char characters[EVA_CLOCK_CHARS] = {
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
    [Fede] = {{1, 2}, {2, 5}, {4, 0}, {5, 3}},
    [Mart] = {{0, 5}, {0, 6}, {0, 7}, {1, 0}},
    [June] = {{4, 6}, {6, 1}, {7, 4}, {8, 7}},
    [Greg] = {{1, 1}, {2, 4}, {3, 7}, {5, 2}},
    [FM] = {{4, 4}, {4, 5}},
};

static clockbits bits[EVA_CLOCK_LAYERS];
SemaphoreHandle_t clock_semaphore = NULL;
StaticSemaphore_t clock_semaphore_buffer;

static const char* TAG = "clock";

void clock_start(void) { clock_semaphore = xSemaphoreCreateMutexStatic(&clock_semaphore_buffer); }

void clock_loop(void*) {
  ESP_LOGI(TAG, "clock_loop starting");

  time_t rawtime;
  struct tm* timeinfo;

  for (;;) {
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    xSemaphoreTake(clock_semaphore, portMAX_DELAY);
    set_time(bits[0], timeinfo);
    set_names(bits[1], timeinfo);
    xSemaphoreGive(clock_semaphore);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  ESP_LOGW(TAG, "clock_loop exited");
}

void clock_get(clockbits copy[EVA_CLOCK_LAYERS]) {
  xSemaphoreTake(clock_semaphore, portMAX_DELAY);
  memcpy(copy, bits, sizeof(bits));
  xSemaphoreGive(clock_semaphore);
}

bool has_bit(clockbits a, int n) {
  int word = n / EVA_CLOCK_WORD_SIZE;
  int bit = (EVA_CLOCK_WORD_SIZE - 1) - (n % EVA_CLOCK_WORD_SIZE);

  return (a[word] >> bit) & 1;
}

void or_bits(clockbits a, clockbits b) {
  for (int i = 0; i < EVA_CLOCK_WORD_COUNT; i++) {
    a[i] |= b[i];
  }
}

void or_leds(clockbits a, const clockleds b) {
  for (int i = 0; i < EVA_CLOCK_LEDS_SIZE; i++) {
    if (b[i][0] == 0 && b[i][1] == 0) {
      continue;
    }

    a[b[i][0]] |= 1 << (7 - b[i][1]);
  }
}

void or_mask(clockbits a, const clockmask* b) {
  for (int i = 0; i < EVA_CLOCK_MASK_SIZE; i++) {
    a[b->offset + i] |= b->words[i];
  }
}

void print_bits(clockbits a) {
  for (int i = 0; i < EVA_CLOCK_CHARS; i++) {
    printf("%d", has_bit(a, i));

    if ((i + 1) % EVA_CLOCK_WIDTH == 0) {
      printf("\n");
    }
  }
}

void print_chars(clockbits a) {
  for (int i = 0; i < EVA_CLOCK_CHARS; i++) {
    bool enabled = has_bit(a, i);

    printf("%c", enabled ? characters[i] : '-');

    if ((i + 1) % EVA_CLOCK_WIDTH == 0) {
      printf("\n");
    }
  }
}

void set_names(clockbits a, struct tm* time) {
  memset(a, 0, sizeof(clockbits));

  if (time->tm_mon == 8 && time->tm_mday == 3) {
    or_leds(a, names[Fede]);
  }

  if (time->tm_mon == 1 && time->tm_mday == 26) {
    or_leds(a, names[Mart]);
  }

  if (time->tm_mon == 3 && time->tm_mday == 1) {
    or_leds(a, names[June]);
  }

  if (time->tm_mon == 0 && time->tm_mday == 15) {
    or_leds(a, names[Greg]);
  }

  if (time->tm_mon == 8 && time->tm_mday == 15) {
    or_leds(a, names[FM]);
  }

  if (EVA_CLOCK_DEBUG) {
    int all[5] = {Fede, Mart, June, Greg, FM};
    int name = all[time->tm_min % 5];
    or_leds(a, names[name]);
  }
}

void set_time(clockbits a, struct tm* time) {
  memset(a, 0, sizeof(clockbits));

  or_mask(a, &words[ItIs]);

  if (time->tm_min < 5) {
    or_mask(a, &words[OClock]);
  } else if ((time->tm_min >= 5 && time->tm_min < 10) || (time->tm_min >= 55)) {
    or_mask(a, &words[FiveMinutes]);
  } else if ((time->tm_min >= 10 && time->tm_min < 15) || (time->tm_min >= 50 && time->tm_min < 55)) {
    or_mask(a, &words[TenMinutes]);
  } else if ((time->tm_min >= 15 && time->tm_min < 20) || (time->tm_min >= 45 && time->tm_min < 50)) {
    or_mask(a, &words[AQuarter]);
  } else if ((time->tm_min >= 20 && time->tm_min < 25) || (time->tm_min >= 40 && time->tm_min < 45)) {
    or_mask(a, &words[TwentyMinutes]);
  } else if ((time->tm_min >= 25 && time->tm_min < 30) || (time->tm_min >= 35 && time->tm_min < 40)) {
    or_mask(a, &words[TwentyMinutes]);
    or_mask(a, &words[FiveMinutes]);
  } else {
    or_mask(a, &words[Half]);
  }

  int hour = time->tm_hour;

  if (time->tm_min >= 5 && time->tm_min < 35) {
    or_mask(a, &words[Past]);
  } else if (time->tm_min >= 35) {
    or_mask(a, &words[To]);
    hour++;
  }

  int hour_mask = Twelve + (hour % 12);
  or_mask(a, &words[hour_mask]);
}
