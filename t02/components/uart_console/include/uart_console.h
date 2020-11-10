#ifndef UART_CONSOLE_H
# define UART_CONSOLE_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_console.h"
#include "esp_err.h"
#include "driver/uart.h"
#include "freertos/semphr.h"
#include "uart_utils_funcs.h"
#include "commands.h"
#include "defines.h"


void uart_event_task(void *pvParams);
void cmd_instance_task(void *pvParams);
int8_t uart_console_init();

#endif
