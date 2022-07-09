#include "eva_clock.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "time.h"

// clang-format off
static const char characters[EVA_CLOCK_CHARS] = {
  'I','T','L','I','S','P','F','E', 'D','E','W',  // 0 - 1
  'E','V','A','C','Q', 'U','A','R','T','E','R',  // 1 - 2
  'T','W', 'E','N','T','Y','X','F','I','V', 'E', // 2 - 3 - 4
  'H','A','L','F','M','A','R', 'T','E','N','J',  // 4 - 5
  'P','A','S','T', 'O','R','N','I','N','E','U',  // 5 - 6
  'E', 'I','G','H','T','E','L','E','V', 'E','N', // 6 - 7 - 8
  'O','N','E','S','I','X', 'T','H','R','E','E',  // 8 - 9
  'S','E','V', 'E','N','T','W','E','L','V','E',  // 9 - 10
  'F','O','U','R','F','I','V','E', 'T','W','O',  // 11 - 12
  'T','E','N','Y','L', 'O','C','L','O','C','K',  // 12 - 13
};
// clang-format on

static const clockmask words[] = {
    [ItIs] = {0, {0b11011000, 0b00000000}},
    [FiveMinutes] = {3, {0b00000111, 0b10000000}},
    [TenMinutes] = {5, {0b11100000, 0b00000000}},
    [AQuarter] = {1, {0b00000101, 0b11111100}},
    [TwentyMinutes] = {2, {0b00000011, 0b11110000}},
    [Half] = {4, {0b01111000, 0b00000000}},
    [To] = {5, {0b00000001, 0b10000000}},
    [Past] = {5, {0b00001111, 0b00000000}},
    [OClock] = {13, {0b11111100, 0b00000000}},
    [Twelve] = {10, {0b00111111, 0b00000000}},
    [One] = {8, {0b00111000, 0b00000000}},
    [Two] = {12, {0b11100000, 0b00000000}},
    [Three] = {9, {0b11111000, 0b00000000}},
    [Four] = {11, {0b11110000, 0b00000000}},
    [Five] = {11, {0b00001111, 0b00000000}},
    [Six] = {8, {0b00000111, 0b00000000}},
    [Seven] = {9, {0b00000111, 0b11000000}},
    [Eight] = {6, {0b00000001, 0b11110000}},
    [Nine] = {6, {0b00111100, 0b00000000}},
    [Ten] = {12, {0b00011100, 0b00000000}},
    [Eleven] = {7, {0b00001111, 0b11000000}},
};

static const clockleds names[] = {
    [Fede] = {{0, 6}, {0, 7}, {1, 0}, {1, 1}},
    [Mart] = {{4, 5}, {4, 6}, {4, 7}, {5, 0}},
    [June] = {{5, 3}, {6, 6}, {8, 1}, {9, 4}},
    [Eva] = {{1, 3}, {1, 4}, {1, 5}},
    [FM] = {{4, 4}, {4, 5}},
};

static clockbits bits[EVA_CLOCK_LAYERS];
SemaphoreHandle_t clock_semaphore = NULL;
StaticSemaphore_t clock_semaphore_buffer;

static const char* TAG = "clock";

void clock_start(void) { clock_semaphore = xSemaphoreCreateMutexStatic(&clock_semaphore_buffer); }

void clock_loop(void* param) {
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

  ESP_LOGI(TAG, "clock_loop exited");
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

  if (true) {
    or_leds(a, names[June]);
  }

  if (time->tm_mon == 0 && time->tm_mday == 15) {
    or_leds(a, names[Eva]);
  }

  if (time->tm_mon == 8 && time->tm_mday == 15) {
    or_leds(a, names[FM]);
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
