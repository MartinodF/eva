#include "eva_wifi.h"

static EventGroupHandle_t wifi_event_group;

static const char* TAG = "wifi";

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI(TAG, "wifi started, connecting...");
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    xEventGroupClearBits(wifi_event_group, EVA_WIFI_CONNECTED_BIT);
    ESP_LOGI(TAG, "disconnected, reconnecting...");
    esp_wifi_connect();
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(wifi_event_group, EVA_WIFI_CONNECTED_BIT);
  }
}

void wifi_start(void) {
  ESP_LOGI(TAG, "wifi_start");

  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = EVA_WIFI_SSID,
              .password = EVA_WIFI_PASS,
              .listen_interval = 10,
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_wifi_set_ps(WIFI_PS_MAX_MODEM);

  ESP_LOGI(TAG, "wifi_start done");
}

bool wifi_available() {
  return (xEventGroupGetBits(wifi_event_group) & EVA_WIFI_CONNECTED_BIT) == EVA_WIFI_CONNECTED_BIT;
}
