#include "wifi_connection.h"

esp_err_t wifi_init()
{
    esp_netif_init();

    esp_err_t err;
    err = esp_event_loop_create_default();

    if (err != ESP_OK)
    {
        printf("%s %sn", "couldn't create default event loop",
               esp_err_to_name(err));
        return (err);
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&wifi_config);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't init wi-fi module", esp_err_to_name(err));
        return (err);
    }

    err = esp_wifi_start();
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't start wifi", esp_err_to_name(err));
        return (err);
    }

    return (ESP_OK);
}

esp_err_t wifi_connect(char *ssid_name, char *passwd)
{
    esp_wifi_set_mode(WIFI_MODE_STA);

    if(strlen(ssid_name) > 0)
    {
        wifi_config_t sta_config = {.sta = {
                                        .bssid_set          = 0,
                                        //.threshold.authmode = WIFI_AUTH_WPA2_PSK,
                                    }};

        memcpy(sta_config.sta.ssid, ssid_name, strlen(ssid_name));
        memcpy(sta_config.sta.password, passwd, strlen(passwd));

        esp_err_t err;

        err = esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config);
        if (err != ESP_OK)
        {
            printf("%s %s\n", "couldn't set sta mode config", esp_err_to_name(err));
        }
        
        uart_print_str_value("\e[33mTry connect to ", ssid_name, "\e[39m");
        uart_clear_line();

        wifi_info_update_ssid(ssid_name, passwd);

        err = esp_wifi_connect();
        if (err != ESP_OK)
        {
            printf("%s %s\n", "couldn't connect", esp_err_to_name(err));
        }

        return (ESP_OK);
    }
    else
    {
        uart_print_str(UART_NUMBER, "ssid doesn't provided. Can't connect");
        uart_clear_line();
        return (ESP_OK);
    }
}

void wifi_display_info()
{
    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);
    uart_print_str(UART_NUMBER, "\n\r");
    if(wifi_sta_info->is_connected)
    {
        uart_print_str(UART_NUMBER, "\e[K\n\r");
        uart_print_str(UART_NUMBER, "sta_state");
        uart_print_str(UART_NUMBER, "\e[K\n\r");
        uart_print_str_value("State: ", wifi_sta_info->state, NULL);
        uart_print_str(UART_NUMBER, "\e[K\n\r");
        uart_print_str_value("SSID: ", wifi_sta_info->ssid_str, NULL);
        uart_print_str(UART_NUMBER, "\e[K\n\r");
        uart_print_uint8t_value("Channel: ", wifi_sta_info->channel, NULL);
        uart_print_str(UART_NUMBER, "\e[K\n\r");
        uart_print_int8t_value("RSSI: ", wifi_sta_info->rssi, " dBm");
        uart_print_str(UART_NUMBER, "\e[K\n\r");

    }
    else
    {
        uart_print_str(UART_NUMBER, "\e[K\n\r");
        uart_print_str_value("State: ", wifi_sta_info->state, NULL);
        uart_print_str(UART_NUMBER, "\e[K\n\r");
    }
    uart_flush_saved_input();   
}

void wifi_success_conn()
{
    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);
}

void wifi_wipe_info()
{
    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    memset(wifi_sta_info->ssid, 0, 32);
    wifi_sta_info->wifi_reconnect_count = 0;
    wifi_sta_info->state = "DISCONNECTED";
    wifi_sta_info->channel = 0;
    wifi_sta_info->rssi = 0;
    wifi_sta_info->ip = NULL;
    wifi_sta_info->is_connected = false;

    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);
}

void wifi_info_update_ssid(char *ssid, char *passwd)
{
    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    wifi_sta_info->ssid_str = ssid;
    wifi_sta_info->passwd = passwd;

    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);
}

char *wifi_ssid_to_str(u_int8_t *ssid)
{   
    char *ssid_str;

    ssid_str = calloc(33, sizeof(char));

    memcpy(ssid_str, ssid, 33);

    return ssid_str;
}

uint8_t *wifi_str_to_ssid(char *ssid_str)
{   
    uint8_t *ssid;

    ssid = calloc(32, sizeof(uint8_t));

    memcpy(ssid, ssid_str, 32);

    return ssid;
}

esp_err_t   wifi_get_nvs_data(wifi_sta_info_s *wifi_sta_info)
{
    nvs_open("wifi_store", NVS_READWRITE, &wifi_nvs_handle);

    char *ssid;
    char *passwd;
    ssid = "";
    passwd = "";

    size_t ssid_len = 0;
    size_t passwd_len = 0;

    esp_err_t wifi_is_ssid_saved;
    esp_err_t wifi_is_passwd_saved;

    wifi_is_ssid_saved = nvs_get_str(wifi_nvs_handle, "wifi_ssid", NULL, &ssid_len);
    wifi_is_passwd_saved = nvs_get_str(wifi_nvs_handle, "wifi_passwd", NULL, &passwd_len);
    
    if(wifi_is_ssid_saved != ESP_OK)
    {
        printf("%s %s\n", "couldn't find nvs",
               esp_err_to_name(wifi_is_ssid_saved));
        return ESP_FAIL;
    }
    
    if(wifi_is_ssid_saved == ESP_OK)
    {
        ssid = (char *)malloc(ssid_len);
        wifi_is_ssid_saved = nvs_get_str(wifi_nvs_handle, "wifi_ssid", ssid, &ssid_len);
        if(wifi_is_ssid_saved != ESP_OK)
        {
            printf("%s\n", "couldn't get nvs ssid value");
            return ESP_FAIL;
        }
    }
    if(wifi_is_passwd_saved == ESP_OK)
    {
        passwd = (char *)malloc(passwd_len);
        wifi_is_passwd_saved = nvs_get_str(wifi_nvs_handle, "wifi_passwd", passwd, &passwd_len);
        if(wifi_is_passwd_saved != ESP_OK)
        {
            printf("%s\n", "couldn't get nvs passwd value");
            return ESP_FAIL;
        }
        
    }
    wifi_sta_info->ssid_str = ssid;
    wifi_sta_info->passwd = passwd;

    nvs_close(wifi_nvs_handle);

    return ESP_OK;
}