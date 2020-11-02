idf_component_register(SRCS "wifi_events.c"
                       INCLUDE_DIRS "include"
                       REQUIRES defines nvs_flash wifi_connection)