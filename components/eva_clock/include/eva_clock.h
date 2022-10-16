#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "time.h"

#define CEILING(x, y) (((x) + (y)-1) / (y))

#define EVA_CLOCK_HEIGHT 10
#define EVA_CLOCK_WIDTH 11
#define EVA_CLOCK_CHARS (EVA_CLOCK_HEIGHT * EVA_CLOCK_WIDTH)
#define EVA_CLOCK_WORD_SIZE 8
#define EVA_CLOCK_WORD_COUNT CEILING(EVA_CLOCK_CHARS, EVA_CLOCK_WORD_SIZE)
#define EVA_CLOCK_MASK_SIZE 2
#define EVA_CLOCK_LAYERS 2
#define EVA_CLOCK_LEDS_SIZE 4

typedef uint8_t clockbits[EVA_CLOCK_WORD_COUNT];

typedef struct clockmask {
  int offset;
  uint8_t words[EVA_CLOCK_MASK_SIZE];
} clockmask;

typedef int clockled[2];
typedef clockled clockleds[EVA_CLOCK_LEDS_SIZE];

enum Words {
  ItIs,
  FiveMinutes,
  TenMinutes,
  AQuarter,
  TwentyMinutes,
  Half,
  OClock,
  To,
  Past,
  Twelve,
  One,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Eleven,
  Fede,
  Mart,
  June,
  Greg,
  FM,
};

void clock_start(void);
void clock_loop(void*);
void clock_get(clockbits[EVA_CLOCK_LAYERS]);
bool has_bit(clockbits, int);
void or_bits(clockbits, clockbits);
void or_leds(clockbits, const clockleds);
void or_mask(clockbits, const clockmask*);
void print_bits(clockbits);
void print_chars(clockbits);
void set_names(clockbits, struct tm*);
void set_time(clockbits, struct tm*);
