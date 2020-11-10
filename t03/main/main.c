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

void app_main()
{
    esp_err_t err;

    gpio_set_direction(DHT_POWER, GPIO_MODE_OUTPUT);	
	gpio_set_level(DHT_POWER, 1);

    nvs_flash_init();

    uart_console_init();
    register_cmnd_set();

    err = wifi_init();
    if(err != ESP_OK)
    {
        printf("%s\n", "WIFI initialization failed");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);

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

    wifi_info_queue = xQueueCreate( 1, sizeof(wifi_sta_info_s) );
    socket_params_queue = xQueueCreate( 1, sizeof(socket_params_s) );
    get_url_params_queue = xQueueCreate( 1, sizeof(get_url_params_s) );
    send_dht_url_queue = xQueueCreate( 1, sizeof(get_url_params_s) );
    //send_dht_done_queue = xQueueCreate( 1, sizeof(bool) );
    send_dht_mutex = xSemaphoreCreateMutex();

    UBaseType_t is_filled = 0;
    is_filled = uxQueueMessagesWaiting(wifi_info_queue);

    if (!is_filled)
    {
        xQueueSend(wifi_info_queue, &wifi_sta_info, 10);
    }

    wifi_connect(wifi_sta_info.ssid_str, wifi_sta_info.passwd);

}
