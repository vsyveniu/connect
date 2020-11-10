#ifndef SOCKET_CONNECTION_H
# define SOCKET_CONNECTION_H

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

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include <errno.h>

esp_err_t socket_set_options(int sock);
esp_err_t dns_find_by_hostname(char* host, ip_addr_t* ip_Addr);
esp_err_t socket_create(int addr_family, int type, int ip_protocol, struct sockaddr_in *dest_addr, int *sock);

#endif
