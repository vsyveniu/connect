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
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

esp_err_t socket_set_options(int sock);
esp_err_t dns_find_by_hostname(char* host, ip_addr_t* ip_Addr);
esp_err_t socket_create(int addr_family, int type, int ip_protocol, struct sockaddr_in *dest_addr, int *sock);
esp_err_t socket_tls_create(char * host, char * query, uint8_t temp, uint8_t hum, char *rx_buff, char* json_str, const char *port);
esp_err_t tls_connect(mbedtls_ssl_context *ssl, char *host, const char *port);
void http_response_print(char* str);

void make_json_payload(char *method, char* payload, char* host, char* query, int json_len, char *json_str);

#endif
