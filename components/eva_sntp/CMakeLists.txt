idf_component_register(SRCS "eva_sntp.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event esp_timer eva_event)
