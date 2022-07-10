#include "esp_log.h"
#include "eva_led.h"
#include "eva_leds.h"
#include "eva_sntp.h"
#include "eva_wifi.h"

#define EVA_STATUS_GPIO 40

void status_loop(void*);
