idf_component_register(SRCS "eva_led.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event eva_display eva_event eva_rmt)
