#include "wifi_events.h"

static void wifi_scan_handler(void *handler_args, esp_event_base_t base,
                              int32_t id, void *event_data)
{
    printf("wifi scan event  %s %d\n", base, id);
}

static void wifi_connect_handler(void *handler_args, esp_event_base_t base,
                                 int32_t id, void *event_stuff)
{
    wifi_event_sta_connected_t *event_data = event_stuff;

    wifi_ap_record_t wifi_ap_info;

    esp_err_t err;

    err = esp_wifi_sta_get_ap_info(&wifi_ap_info);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't get ap info", esp_err_to_name(err));
    }

    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    memcpy(wifi_sta_info->ssid, event_data->ssid, 32);
    wifi_sta_info->ssid_str             = wifi_ssid_to_str(event_data->ssid);
    wifi_sta_info->channel              = event_data->channel;
    wifi_sta_info->state                = "CONNECTED";
    wifi_sta_info->rssi                 = wifi_ap_info.rssi;
    wifi_sta_info->is_connected         = true;
    wifi_sta_info->wifi_reconnect_count = 0;

    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);
}

static void wifi_timeout_handler(void *handler_args, esp_event_base_t base,
                                 int32_t id, void *event_stuff)
{
    printf("%s\n", "wifi connection timed out");
}

// wifi disconnect handling
void wifi_divorce_by_intention()
{
    wifi_wipe_info();
}

void wifi_divorce_no_ap(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value("\e[31mcould'nt find access point,  err: \e[91m",
                            event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_divorce_assoc(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value("\e[31assoc fail,  err:  \e[91m",
                            event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_divorce_assoc_expire(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value("\e[31massoc expire,  err: \e[91m",
                            event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_divorce_assoc_many(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value("\e[31maccos too many,  err: \e[91m",
                            event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_divorce_auth_fail(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value(
        "\e[31mauthorization fail, bad password? err: \e[91m",
        event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_divorce_beacon_timeout(wifi_event_sta_disconnected_t *event_data)
{
    wifi_wipe_info();
    uart_clear_up_line();
    uart_print_uint8t_value(
        "\e[31mbeacon timeout, wifi is dead, hope he didn't suffer err: \e[91m",
        event_data->reason, "\e[39m");
    uart_clear_line();
}

void wifi_divorce_auth_expire(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value("\e[31mauthorization expire,  err: \e[91m",
                            event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_divorce_4way_handsahke(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value(
        "\e[31m4 way handsahke fail, perhaps you have entered wrong password?  "
        "err: \e[91m",
        event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_divorce_handsahke_timeout(wifi_event_sta_disconnected_t *event_data)
{
    uart_clear_up_line();
    uart_print_uint8t_value(
        "\e[31mhandsahke timeout, perhaps you have entered wrong password?  "
        "err: ",
        event_data->reason, "\e[39m");
    uart_clear_line();
    wifi_try_reconnect();
}

void wifi_try_reconnect()
{
    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    if (wifi_sta_info->wifi_reconnect_count < WIFI_RECONNECT_MAX / 2)
    {
        wifi_connect(wifi_sta_info->ssid_str, wifi_sta_info->passwd);
    }
    else
    {
        if (wifi_sta_info->fallback_ssid != NULL &&
            wifi_sta_info->wifi_reconnect_count < WIFI_RECONNECT_MAX)
        {
            uart_clear_up_line();
            uart_print_str(
                UART_NUMBER,
                "Gonna try to connect to previous successfull access point");
            uart_clear_line();

            // set passwd as fallback cause if not and successfully reconenct to
            // fallback, the passwd field will be incorrect from cli passwd
            // update
            wifi_sta_info->passwd = wifi_sta_info->fallback_passwd;
            wifi_connect(wifi_sta_info->fallback_ssid,
                         wifi_sta_info->fallback_passwd);
        }
        else
        {
            wifi_wipe_info();
            uart_print_str(UART_NUMBER,
                           "\rcouldn't connect to any access point\n\r");
        }
    }

    wifi_sta_info->wifi_reconnect_count++;
    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);
    uart_flush_saved_input();
}

static void wifi_disconnect_handler(void *handler_args, esp_event_base_t base,
                                    int32_t id, void *event_stuff)
{
    wifi_event_sta_disconnected_t *event_data = event_stuff;

    /// uart_print_str(UART_NUMBER, "\n\n\r");
    switch (event_data->reason)
    {
    case WIFI_REASON_ASSOC_LEAVE:
        wifi_divorce_by_intention();
        break;
    case WIFI_REASON_NO_AP_FOUND:
        wifi_divorce_no_ap(event_data);
        break;
    case WIFI_REASON_AUTH_FAIL:
        wifi_divorce_auth_fail(event_data);
        break;
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        wifi_divorce_4way_handsahke(event_data);
        break;
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
        wifi_divorce_handsahke_timeout(event_data);
        break;
    case WIFI_REASON_ASSOC_FAIL:
        wifi_divorce_assoc(event_data);
        break;
    case WIFI_REASON_ASSOC_EXPIRE:
        wifi_divorce_assoc_expire(event_data);
        break;
    case WIFI_REASON_AUTH_EXPIRE:
        wifi_divorce_auth_expire(event_data);
        break;
    case WIFI_REASON_ASSOC_TOOMANY:
        wifi_divorce_assoc_many(event_data);
        break;
    case WIFI_REASON_BEACON_TIMEOUT:
        wifi_divorce_beacon_timeout(event_data);
        break;
    default:
        wifi_try_reconnect();
        break;
    }
}

// end

static void wifi_got_ip_handler(void *handler_args, esp_event_base_t base,
                                int32_t id, void *event_stuff)
{

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_stuff;

    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    // size of ipv4 - 4 bytes ipv6 - 128. Set buff of size capable to store ipv6
    // converted to string by IP2STR
    char ip_buff[16];
    memset(ip_buff, 0, 16);

    sprintf(ip_buff, IPSTR, IP2STR(&event->ip_info.ip));

    nvs_open("wifi_store", NVS_READWRITE, &wifi_nvs_handle);
    esp_err_t err;
    err = nvs_set_str(wifi_nvs_handle, "wifi_ssid", wifi_sta_info->ssid_str);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't find nvs key", esp_err_to_name(err));
    }
    err = nvs_set_str(wifi_nvs_handle, "wifi_passwd", wifi_sta_info->passwd);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't find nvs key", esp_err_to_name(err));
    }
    err = nvs_commit(wifi_nvs_handle);
    if (err != ESP_OK)
    {
        printf("%s", esp_err_to_name(err));
    }
    nvs_close(wifi_nvs_handle);

    wifi_sta_info->ip              = ip_buff;
    wifi_sta_info->fallback_ssid   = wifi_sta_info->ssid_str;
    wifi_sta_info->fallback_passwd = wifi_sta_info->passwd;

    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);

    uart_print_str(UART_NUMBER, "\r");
    uart_print_str_value("\e[32mSuccess! \e[39mGot IP: \e[34m",
                         wifi_sta_info->ip, "\e[39m");
    wifi_display_info();
}

esp_err_t wifi_register_events()
{
    esp_err_t err;

    err = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,
                                              wifi_scan_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't register event instance",
               esp_err_to_name(err));
        return (err);
    }

    err = esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, wifi_connect_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't register event instance",
               esp_err_to_name(err));
        return (err);
    }

    err = esp_event_handler_instance_register(WIFI_EVENT,
                                              WIFI_EVENT_STA_WPS_ER_TIMEOUT,
                                              wifi_timeout_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't register event instance",
               esp_err_to_name(err));
        return (err);
    }

    err = esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_disconnect_handler, NULL,
        NULL);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't register event instance",
               esp_err_to_name(err));
        return (err);
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                              wifi_got_ip_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't register event instance",
               esp_err_to_name(err));
        return (err);
    }

    return (ESP_OK);
}