idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "include"
                        REQUIRES defines nvs_flash uart_console wifi_connection wifi_events
                      )