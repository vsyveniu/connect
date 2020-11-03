#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_console.h"
#include "esp_err.h"
#include "uart_utils_funcs.h"
#include "driver/ledc.h"
#include "defines.h"
#include "commands_handle.h"

void    register_cmnd_set();
int     cmd_ssid_set(int argc, char **argv);
int     cmd_connection_status(int argc, char **argv);
int     cmd_disconnect(int argc, char **argv);
int     cmd_help(int argc, char **argv);
int     cmd_scan(int argc, char **argv);
int     cmd_sock_ping(int argc, char** argv);
int     cmd_exit();

#endif
