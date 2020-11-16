#include "commands_handle.h"
#include "driver/periph_ctrl.h"
#include "soc/timer_group_struct.h"
#include <sys/time.h>
#include "sntp.h"
#include "argtable3/argtable3.h"

//#include <sys/socket.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include <errno.h>
#include "socket_connection.h"
#include "esp_tls.h"
#include "esp_wifi.h"

static const char* ERRORTAG = "error: ";
static const char* INFOTAG = "info: ";
TaskHandle_t send_dht_handle = NULL;

void handle_ssid_set(struct arg_str* ssid, struct arg_str* passwd)
{
    int32_t ssid_len;
    int32_t passwd_len;

    ssid_len = strlen(*ssid->sval);
    passwd_len = strlen(*passwd->sval);

    char* ssid_copy;
    char* passwd_copy;

    ssid_copy = calloc(ssid_len + 1, sizeof(char));
    passwd_copy = calloc(passwd_len + 1, sizeof(char));

    memcpy(ssid_copy, *ssid->sval, ssid_len);
    memcpy(passwd_copy, *passwd->sval, passwd_len);

    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    wifi_sta_info->ssid_str = ssid_copy;
    wifi_sta_info->passwd = passwd_copy;
    wifi_sta_info->wifi_reconnect_count = 0;

    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);

    esp_wifi_disconnect();

    uart_print_str(UART_NUMBER, "\r\n");
    wifi_connect(ssid_copy, passwd_copy);
}

void handle_alarm(struct arg_rex* alarm_set)
{
    uint64_t hours = 0;
    uint64_t minutes = 0;
    uint64_t seconds = 0;
    uint64_t alarm_val = 0;
    UBaseType_t is_filled = 0;

    hours = atoi(*alarm_set->sval);
    *alarm_set->sval += 3;
    minutes = atoi(*alarm_set->sval);
    *alarm_set->sval += 3;
    seconds = atoi(*alarm_set->sval);

    alarm_val = ((hours * 3600) + minutes * 60 + seconds);

    is_filled = uxQueueMessagesWaiting(alarm_queue);

    if (!is_filled)
    {
        xQueueSend(alarm_queue, &alarm_val, 10);
    }
    else
    {
        xQueueOverwrite(alarm_queue, &alarm_val);
    }
}


void handle_connection_status() { wifi_display_info(); }

void handle_disconnect() { esp_wifi_disconnect(); }


void handle_help()
{
    uart_print_str(UART_NUMBER, "\n\rExamples of commands you may use:");
    uart_print_str(UART_NUMBER, "\n\rconnect -s=AP ssid  -p=password");
    uart_print_str(UART_NUMBER, "\n\rconnect -s AP ssid  -p password");
    uart_print_str(UART_NUMBER, "\n\rconnection-status");
    uart_print_str(UART_NUMBER, "\n\rdisconnect");
}