#pragma once

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define EVA_WIFI_SSID "Connecto Patronum"
#define EVA_WIFI_PASS "fedemart"
#define EVA_WIFI_CONNECTED_BIT BIT0

void wifi_start(void);
bool wifi_available(void);
