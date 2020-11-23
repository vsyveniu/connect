idf_component_register(SRCS "http_server.c"
                       INCLUDE_DIRS "include"
                       REQUIRES defines nvs_flash esp_http_server spiffs wifi_connection)