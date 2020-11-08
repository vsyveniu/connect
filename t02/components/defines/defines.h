#ifndef DEFINES_H
#define DEFINES_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"

#define UART_NUMBER        UART_NUM_1
#define UART_TX_PIN        17
#define UART_RX_PIN        16
#define WIFI_RECONNECT_MAX 8

typedef struct wifi_sta_info_t
{
    uint8_t ssid[32];
    int8_t  wifi_reconnect_count;
    int8_t  rssi;
    uint8_t channel;
    char    *state;
    char    *ssid_str;
    char    *passwd;
    char    *fallback_ssid;
    char    *fallback_passwd;
    char    *ip;
    bool    is_connected;
    
} wifi_sta_info_s;

typedef struct uart_saved_input_t
{
    char *p_saved;
        
} uart_saved_input_s;

typedef struct socket_params_t
{
    int32_t     port;
    int32_t     count;
    char        *ip;
        
} socket_params_s;

typedef struct get_url_params_t
{
    const char *host;
    const char *query;

} get_url_params_s;

QueueHandle_t wifi_info_queue;
QueueHandle_t uart_save_input_queue;
QueueHandle_t uart_is_saved;
QueueHandle_t socket_params_queue;
QueueHandle_t get_url_params_queue;
xSemaphoreHandle uart_mutex_output;
nvs_handle_t wifi_nvs_handle;

#endif