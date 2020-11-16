idf_component_register(SRCS "socket_connection.c"
                       INCLUDE_DIRS "include"
                       REQUIRES defines nvs_flash uart_utils_funcs esp-tls)