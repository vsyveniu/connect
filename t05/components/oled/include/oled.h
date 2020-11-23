#ifndef OLED_H
# define OLED_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <string.h>
#include <inttypes.h>
#include "driver/i2c.h"
#include "defines.h"
#include <stdarg.h>

void display_str(char *str, int page, int appear_speed, int font_weight);
int display_str_fat_row_2(char *str, int appear_speed, int font_weight, int page_start, int pages_num);
void create_load(uint8_t *arr, char *str, int len, int font_weight);
void write_page(uint8_t *data, uint8_t page);
int32_t init_oled();
void clear_oled();
void fill_oled();
void reconfigure_oled(int8_t num, ...);

#endif
