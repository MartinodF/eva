idf_component_register(SRCS "eva_wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event esp_timer esp_wifi eva_event)
