#ifndef TIMER_UTILS_H
# define TIMER_UTILS_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include <unistd.h>
#include "defines.h"
#include "uart_utils_funcs.h"

char *make_time_str(uint8_t hours, uint8_t minutes, uint8_t seconds);
esp_err_t sntp_time_set(char *timezone);
esp_err_t timer_0_init();

#endif
