#ifndef WIFI_CONNECTION_H
# define WIFI_CONNECTION_H

#include "defines.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include <string.h>
#include <unistd.h>
#include "uart_utils_funcs.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"

esp_err_t wifi_init();
esp_err_t wifi_connect(char *ssid_name, char *passwd);
esp_err_t wifi_scan_aps();
void wifi_display_info();
void wifi_wipe_info();
void wifi_info_update_ssid(char *ssid, char *passwd);
char *wifi_ssid_to_str(u_int8_t *ssid);
uint8_t *wifi_str_to_ssid(char *ssid_str);
esp_err_t   wifi_get_nvs_data(wifi_sta_info_s *wifi_sta_info);
void wifi_full_wipe_info();
esp_err_t wifi_scan_aps();

#endif
