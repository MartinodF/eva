#pragma once

#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "eva_event.h"

#define CEILING(x, y) (((x) + (y)-1) / (y))

#define EVA_DISPLAY_HEIGHT 10
#define EVA_DISPLAY_WIDTH 11
#define EVA_DISPLAY_PIXELS (EVA_DISPLAY_HEIGHT * EVA_DISPLAY_WIDTH)
#define EVA_DISPLAY_WORD_SIZE 8
#define EVA_DISPLAY_WORD_COUNT CEILING(EVA_DISPLAY_PIXELS, EVA_DISPLAY_WORD_SIZE)

enum Layer { LayerBooting, LayerClock, LayerEvents, LayerStatus, LayerLight, LayerTemp, LayersCount };
enum Pixel {
  Off = 0,
  White = (1 << 0),
  Rainbow = (1 << 1),
};

typedef uint8_t frame[EVA_DISPLAY_WORD_COUNT];
typedef frame buffer[LayersCount];

typedef int pixels[EVA_DISPLAY_PIXELS];

typedef struct frame_event {
  int layer;
  frame mem;
  bool skip_emit;
} frame_event_t;

void display_start(void);
