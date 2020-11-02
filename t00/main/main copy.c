/** @file main.c *
 * entry point
 */

#include "main.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "uart_console.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"

static EventGroupHandle_t s_wifi_event_group;



static void wifi_scan_handler(void* handler_args, esp_event_base_t base,
                              int32_t id, void* event_data)
{
    printf("wifi scan event  %s %d\n", base, id);
}

static void wifi_connect_handler(void* handler_args, esp_event_base_t base,
                                 int32_t id, void* event_stuff)
{
    wifi_event_sta_connected_t *event_data = event_stuff;
    if(base == IP_EVENT)
    {
        printf("\n%s\n", "FUCK");
    }
    printf("wifi connect event  %s %d, %u \n", base, id, *event_data->ssid);
}

static void wifi_timeout_handler(void* handler_args, esp_event_base_t base,
                                 int32_t id, void* event_stuff)
{
    printf("%s\n", "wifi connection timed out");
}

static void wifi_disconnect_handler(void* handler_args, esp_event_base_t base,
                                    int32_t id, void* event_stuff)
{
    wifi_event_sta_disconnected_t *event_data = event_stuff;
    printf("%s\n", "wifi disconnected");
    printf("wifi disconnect reason  %s, %u \n", base, event_data->reason);
}

static void got_ip_handler(void* handler_args, esp_event_base_t base,
                           int32_t id, void* event_stuff)
{
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_stuff;

    xEventGroupSetBits(s_wifi_event_group, BIT0);
    printf("\n%s\n", "FUCK");
    ESP_LOGI("T", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
}

void app_main()
{
    esp_err_t err;

    nvs_flash_init();

    uart_console_init();

    register_cmnd_set();

    esp_netif_init();
    esp_netif_create_default_wifi_sta(); 


    s_wifi_event_group = xEventGroupCreate();

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&wifi_config);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't init wi-fi module", esp_err_to_name(err));
    }

    esp_wifi_set_mode(WIFI_MODE_STA);
    // esp_wifi_set_storage(WIFI_STORAGE_RAM);

    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't create default event loop",
               esp_err_to_name(err));
    }

     err = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,
                                              wifi_scan_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't register event instance",
               esp_err_to_name(err));
    }

    err = esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, wifi_connect_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't register event instance",
               esp_err_to_name(err));
    }

    err = esp_event_handler_instance_register(WIFI_EVENT,
                                              WIFI_EVENT_STA_WPS_ER_TIMEOUT,
                                              wifi_timeout_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't register event instance",
               esp_err_to_name(err));
    }

    err = esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_disconnect_handler, NULL,
        NULL);
    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't register event instance",
               esp_err_to_name(err));
    } 

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                              wifi_connect_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't register event instance",
               esp_err_to_name(err));
    }

    esp_wifi_start();

    wifi_scan_config_t wifi_scan_conf = {
        .ssid = NULL,
        .bssid = NULL,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
    };

    err = esp_wifi_scan_start(&wifi_scan_conf, true);
    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't scan", esp_err_to_name(err));
    }

    uint16_t aps_count = 0;

    esp_wifi_scan_get_ap_num(&aps_count);

    printf("aps count %u\n", aps_count);

    wifi_ap_record_t* p_wifi_records_list =
        (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * aps_count);

    err = esp_wifi_scan_get_ap_records(&aps_count, p_wifi_records_list);
    if (err != ESP_OK)
    {
        printf("%s %sn\n", "couldn't push records", esp_err_to_name(err));
    }

    int8_t i;

    for (i = 0; i < aps_count; i++)
    {
        char* auth_mode;

        switch (p_wifi_records_list[i].authmode)
        {
        case WIFI_AUTH_OPEN:
            auth_mode = "WIFI_AUTH_OPEN";
            break;
        case WIFI_AUTH_WEP:
            auth_mode = "WIFI_AUTH_WEP";
            break;
        case WIFI_AUTH_WPA_PSK:
            auth_mode = "WIFI_AUTH_WPA_PSK";
            break;
        case WIFI_AUTH_WPA2_PSK:
            auth_mode = "WIFI_AUTH_WPA2_PSK";
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            auth_mode = "WIFI_AUTH_WPA_WPA2_PSK";
            break;
        default:
            auth_mode = "Unknown";
            break;
        }
        printf("ssid = %s, rssi = %d, authmode = %s\n",
               p_wifi_records_list[i].ssid, p_wifi_records_list[i].rssi,
               auth_mode);
    }
    free(p_wifi_records_list);

    wifi_config_t sta_config = {
        .sta = {
            .ssid = "ucode student",
            .password = ">#ucodeworld", 
            .bssid_set = 0,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't set sta mode config", esp_err_to_name(err));
    }

    err = esp_wifi_connect();
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't connect", esp_err_to_name(err));
    }

    esp_netif_ip_info_t *ip_inf;
    esp_netif_get_ip_info(&ip_inf);
}
