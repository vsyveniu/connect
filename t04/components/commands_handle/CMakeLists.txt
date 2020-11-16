idf_component_register(SRCS "commands_handle.c"
                       INCLUDE_DIRS "include"
                       REQUIRES defines console uart_utils_funcs commands wifi_connection socket_connection )