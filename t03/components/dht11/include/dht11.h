#ifndef DHT_H
# define DHT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <esp32/rom/ets_sys.h>
#include "esp_err.h"
#include "defines.h"

int get_DHT_data(uint8_t *dataBytes);

#endif