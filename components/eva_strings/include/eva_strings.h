#pragma once

#include "esp_event.h"
#include "eva_display.h"
#include "eva_event.h"
#include "eva_light.h"
#include "eva_status.h"
#include "eva_temp.h"

enum Mode { Hi, Status, Light, Temp, ModeCount };

void strings_start();
