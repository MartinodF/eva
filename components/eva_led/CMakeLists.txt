idf_component_register(SRCS "eva_led.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_event eva_display eva_event led_strip)
