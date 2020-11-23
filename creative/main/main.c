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

static xQueueHandle butt_queue;

static void IRAM_ATTR butt1_handler(void* args)
{
    uint32_t pin = (uint32_t)args;
    xQueueSendFromISR(butt_queue, &pin, NULL);
}

void butt1_pushed()
{
    uint32_t pin;
    int count = 0;

    while (true)
    {
        if (xQueueReceive(butt_queue, &pin, portMAX_DELAY))
        {
             gpio_intr_disable(pin);    
            count = 0;
            while(gpio_get_level(pin) != 1 && count < 10){
				if(gpio_get_level(pin) == 0){
					vTaskDelay(10/portTICK_PERIOD_MS);
					count = 0;
				}
				else if(gpio_get_level(pin) == 1){
					count++;
				}				
			}

            wifi_switch_params_s wifi_switch_params[1];
            UBaseType_t is_filled = 0;

            is_filled = uxQueueMessagesWaiting(wifi_switch_queue);

            if (is_filled)
            {
                xQueuePeek(wifi_switch_queue, &wifi_switch_params, portMAX_DELAY);

                esp_wifi_disconnect();
                wifi_connect(wifi_switch_params->ssid_str, wifi_switch_params->passwd);
            }
            else
            {
               uart_clear_up_line();
               uart_print_str(UART_NUMBER, "Ssid doesn't provided, use set-wifi-params to store it without provisioning");
               uart_clear_line();
            }
            gpio_intr_enable(pin);
        }
    }
}

void app_main()
{
    esp_err_t err;

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    nvs_flash_init();

    uart_console_init();
    register_cmnd_set();

    gpio_config_t butt_1_conf = {
        .pin_bit_mask = GPIO_SEL_39,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    gpio_config(&butt_1_conf);

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
    wifi_switch_queue = xQueueCreate(1, sizeof(wifi_switch_params_s));

    UBaseType_t is_filled = 0;
    is_filled = uxQueueMessagesWaiting(wifi_info_queue);

    if (!is_filled)
    {
        xQueueSend(wifi_info_queue, &wifi_sta_info, 10);
    }
    get_url_params_queue = xQueueCreate( 1, sizeof(get_url_params_s) );
    wifi_scan_queue = xQueueCreate( 1, sizeof(char) * 1024);

    butt_queue = xQueueCreate(1, sizeof(int));

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTT_1, butt1_handler, (void*)BUTT_1);

    xTaskCreate(butt1_pushed, "button 1 task", 2048, NULL, 1, NULL);

    wifi_scan_aps();
    wifi_connect(wifi_sta_info.ssid_str, wifi_sta_info.passwd);

    esp_netif_create_default_wifi_ap();
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    http_server_init();

}
