#ifndef HTTP_SERVER_H
# define HTTP_SERVER_H

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
#include "esp_http_server.h"

httpd_handle_t http_server_init(void);

#endif
