#include "eva_wifi.h"

static esp_timer_handle_t wifi_disconnected_timer;

static const char* TAG = "wifi";

static void wifi_disconnect_cb(void* param) {
  ESP_LOGW(TAG, "reporting wifi disconnected");
  ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_WIFI_UNHEALTHY, NULL, 0, portMAX_DELAY));
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI(TAG, "wifi started, connecting...");
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "disconnected, reconnecting...");
    esp_wifi_connect();
    esp_timer_stop(wifi_disconnected_timer);
    ESP_ERROR_CHECK(esp_timer_start_once(wifi_disconnected_timer, 5000000ULL));  // 5 seconds
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_WIFI_HEALTHY, NULL, 0, portMAX_DELAY));
    esp_timer_stop(wifi_disconnected_timer);
  }
}

void wifi_start(void) {
  ESP_LOGI(TAG, "wifi_start");

  ESP_ERROR_CHECK(esp_netif_init());

  esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  const esp_timer_create_args_t timer_args = {.callback = &wifi_disconnect_cb, .name = "wifi-disconnect"};
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &wifi_disconnected_timer));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = EVA_WIFI_SSID,
              .password = EVA_WIFI_PASS,
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  ESP_LOGI(TAG, "wifi_start done");
}
