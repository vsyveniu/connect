/** @file main.c *
 * entry point
 */

#include "main.h"
#include "esp_wifi.h"
#include "uart_console.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include "lwip/apps/sntp.h"
#include "wifi_connection.h"
#include "esp_err.h"
#include "http_server.h"

/* static const char* ERRORTAG = "error: ";
static const char* INFOTAG = "info: "; */

void app_main()
{
    esp_err_t err;

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    nvs_flash_init();

    uart_console_init();
    register_cmnd_set();

    scan_mutex = xSemaphoreCreateMutex();

    err = wifi_init();
    if (err != ESP_OK)
    {
        printf("%s\n", "WIFI initialization failed");
    }

    wifi_register_events();

    wifi_sta_info_s wifi_sta_info = {
        .wifi_reconnect_count = 0,
        .state = "DISCONNECTED",
        .ssid = "",
        .passwd = "",
        .ssid_str = "",
        .fallback_ssid = "",
        .fallback_passwd = "",
        .channel = 0,
        .rssi = 0,
        .ip = NULL,
        .is_connected = false,
    };

    wifi_get_nvs_data(&wifi_sta_info);

    wifi_info_queue = xQueueCreate(1, sizeof(wifi_sta_info_s));

    UBaseType_t is_filled = 0;
    is_filled = uxQueueMessagesWaiting(wifi_info_queue);

    if (!is_filled)
    {
        xQueueSend(wifi_info_queue, &wifi_sta_info, 10);
    }
    get_url_params_queue = xQueueCreate( 1, sizeof(get_url_params_s) );
    wifi_scan_queue = xQueueCreate( 1, sizeof(char) * 1024);

    wifi_scan_aps();
    wifi_connect(wifi_sta_info.ssid_str, wifi_sta_info.passwd);

    esp_netif_create_default_wifi_ap();
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    http_server_init();
}
