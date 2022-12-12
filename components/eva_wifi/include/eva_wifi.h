#pragma once

#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "eva_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define EVA_WIFI_SSID "Connecto Patronum"
#define EVA_WIFI_PASS "fedemart"

void wifi_start(void);
