#ifndef DEFINES_H
#define DEFINES_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "freertos/timers.h"
#include "driver/timer.h"

#define UART_NUMBER         UART_NUM_1
#define UART_TX_PIN         17
#define UART_RX_PIN         16
#define WIFI_RECONNECT_MAX  8
#define BUTT_1 	            GPIO_NUM_39


typedef struct wifi_sta_info_t
{
    uint8_t ssid[32];
    int8_t wifi_reconnect_count;
    int8_t rssi;
    uint8_t channel;
    char* state;
    char ssid_str[33];
    char passwd[64];
    char fallback_ssid[33];
    char fallback_passwd[64];
    char* ip;
    bool is_connected;

} wifi_sta_info_s;

typedef struct wifi_switch_params_t
{
    char ssid_str[33];
    char passwd[64];

} wifi_switch_params_s;

typedef struct uart_saved_input_t
{
    char* p_saved;

} uart_saved_input_s;

typedef struct get_url_params_t
{
    char* host;
    char* query;

} get_url_params_s;

QueueHandle_t wifi_info_queue;
QueueHandle_t wifi_switch_queue;
QueueHandle_t uart_save_input_queue;
QueueHandle_t uart_is_saved;
xSemaphoreHandle uart_mutex_output;
xSemaphoreHandle scan_mutex;
nvs_handle_t wifi_nvs_handle;
QueueHandle_t get_url_params_queue;
QueueHandle_t wifi_scan_queue;

#endif