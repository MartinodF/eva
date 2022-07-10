#pragma once

#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "freertos/event_groups.h"

#define EVA_SNTP_HEALTHY_BIT BIT0
#define EVA_SNTP_INTERVAL 3600000ULL  // 1 hour
#define EVA_TIMESERVER "time.google.com"
#define EVA_TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

void sntp_start(void);
void sntp_sync_cb(struct timeval *);
void sntp_timer_cb(void *);
bool sntp_healthy();
