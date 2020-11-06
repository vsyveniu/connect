#ifndef COMMANDS_HANDLE_H
# define COMMANDS_HANDLE_H

#include <unistd.h>
#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <string.h>
#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_console.h"
#include "commands.h"
#include "uart_utils_funcs.h"
#include "freertos/semphr.h"
#include "defines.h"
#include "argtable3/argtable3.h"
#include "wifi_connection.h"

void handle_ssid_set(struct arg_str *ssid, struct arg_str *passwd);
void handle_connection_status();
void handle_disconnect();
void handle_sock_ping(char *ip, int port, int count);
void handle_http_get(const char *host, const char *url);
void handle_help();

#endif