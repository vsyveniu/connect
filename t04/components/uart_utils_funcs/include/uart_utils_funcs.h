#ifndef UART_UTILS_FUNCS_H
#define UART_UTILS_FUNCS_H

#include "defines.h"
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "driver/uart.h"

void uart_print_str(int uart_num, char *str);
void uart_print_int8t_value(char *before, int8_t value, char *after);
void uart_print_int_value(char *before, int value, char *after);
void uart_print_uint8t_value(char *before, uint8_t value, char *after);
void uart_print_str_value(char *before, char *value, char *after);
void uart_flush_saved_input();
void uart_clear_line();
void uart_clear_up_line();


#endif
