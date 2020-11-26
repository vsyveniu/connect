#include "commands_handle.h"
#include "driver/periph_ctrl.h"
#include "soc/timer_group_struct.h"
#include <sys/time.h>
#include "sntp.h"
#include "argtable3/argtable3.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include <errno.h>
#include "socket_connection.h"

static const char* ERRORTAG = "error: ";
static const char* INFOTAG = "info: ";

void handle_ssid_set(struct arg_str* ssid, struct arg_str* passwd)
{
    int32_t ssid_len;
    int32_t passwd_len;

    ssid_len = strlen(*ssid->sval);
    passwd_len = strlen(*passwd->sval);

    char* ssid_copy;
    char* passwd_copy;

    ssid_copy = calloc(ssid_len + 1, sizeof(char));
    passwd_copy = calloc(passwd_len + 1, sizeof(char));

    memcpy(ssid_copy, *ssid->sval, ssid_len);
    memcpy(passwd_copy, *passwd->sval, passwd_len);

    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    wifi_sta_info->ssid_str = ssid_copy;
    wifi_sta_info->passwd = passwd_copy;
    wifi_sta_info->wifi_reconnect_count = 0;

    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);

    esp_wifi_disconnect();

    uart_print_str(UART_NUMBER, "\r\n");
    wifi_connect(ssid_copy, passwd_copy);
}


void http_get_task()
{
    get_url_params_s url_params[1];

    if (xQueueReceive(get_url_params_queue, &url_params, portMAX_DELAY))
    {
        esp_err_t err = 0;
        ip_addr_t ip_Addr;

        err = dns_find_by_hostname(url_params->host, &ip_Addr);
        if (err != ESP_OK)
        {
             ESP_LOGE(ERRORTAG, "couldn't find ip by hostname");
        }
        else
        {
            char* addr = ip4addr_ntoa(&ip_Addr.u_addr.ip4);

            struct sockaddr_in dest_addr = {
                .sin_addr.s_addr = inet_addr(addr),
                .sin_family = AF_INET,
                .sin_port = htons(80),
            };

            inet_ntoa_r(dest_addr.sin_addr, addr, sizeof(addr) - 1);

            int sock;

            err = socket_create(AF_INET, SOCK_STREAM, IPPROTO_IP, &dest_addr, &sock);

            if (err == ESP_OK)
            {
                ESP_LOGI(INFOTAG, "Successfully connected");

                err = socket_set_options(sock);
                if(err != ESP_OK)
                {
                    ESP_LOGE(ERRORTAG, "Unable to set socket options");
                }

                char payload[1024];
                char rx_buffer[1024];

                if (strlen(url_params->query) > 0)
                {
                    sprintf(payload, "GET %s HTTP/1.0\r\nHost: %s\r\n",
                            url_params->query, url_params->host);
                }
                else
                {
                    sprintf(payload, "GET / HTTP/1.0\r\n\r\n");
                }

                err = send(sock, payload, strlen(payload), 0);

                if (err < 0)
                {
                    ESP_LOGE(ERRORTAG,
                             "Error occurred during sending: errno %d", errno);
                }

                int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);

                if (len < 0)
                {
                    ESP_LOGE(ERRORTAG, "recv failed: errno %d", errno);
                }
                else
                {
                    rx_buffer[len] = 0;

                    char *pos = strchr(rx_buffer, '<');

                    if(pos != NULL)
                    {
                        pos--;
                        *pos = '\0';
                        pos++;
                    }
                    else
                    {
                        pos = "";
                    }

                    uart_print_str_value("\r\n\e[32mHEADERS:\n\r\e[34m--------------\e[39m \n\r", rx_buffer, NULL);    
                    uart_print_str_value("\r\n\e[32mPAYLOAD::\n\r\e[34m--------------\e[39m \n\r", pos, NULL);    
                    uart_clear_line();
                }
            } 

            if (sock != -1)
            {
                ESP_LOGI(INFOTAG, "Shutting down socket");
                shutdown(sock, 0);
                close(sock);
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    free(url_params->host);
    vTaskDelete(NULL);
}


void handle_http_get(char* host, char* query)
{
    get_url_params_s url_params[1];

    url_params->host = host, url_params->query = query,

    xQueueSend(get_url_params_queue, &url_params, portMAX_DELAY);

    xTaskCreate(http_get_task, "get request", 4096, NULL, 1, NULL);
}


void handle_connection_status() { wifi_display_info(); }

void handle_disconnect() { esp_wifi_disconnect(); }

void wifi_ping_task(void* params)
{
    socket_params_s socket_params[1];

    if (xQueueReceive(socket_params_queue, &socket_params, portMAX_DELAY))
    {
        char addr_str[128];
        int addr_family;
        int ip_protocol;

         memset(addr_str, 0, 128);

        struct sockaddr_in dest_addr = {
            .sin_addr.s_addr = inet_addr(socket_params->ip),
            .sin_family = AF_INET,
            .sin_port = htons(socket_params->port),

        };

        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        esp_err_t err;
        int32_t i = 0;
        int32_t j = socket_params->count;
        char rx_buffer[128];

        while (j != 0)
        {
            int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
            if (sock < 0)
            {
                ESP_LOGE(ERRORTAG, "Unable to create socket: errno %d", errno);
            }
            ESP_LOGI(INFOTAG, "Socket created, connecting to %s:%d",
                     socket_params->ip, socket_params->port);

            err =
                connect(sock, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
            if (err != 0)
            {
                ESP_LOGE(ERRORTAG, "Socket unable to connect: errno %d", errno);
                break;
            }
            else
            {
                char payload[11];
                sprintf(payload, "PING #%d", i);

                err = send(sock, payload, strlen(payload), 0);
                if (err < 0)
                {
                    ESP_LOGE(ERRORTAG,
                             "Error occurred during sending: errno %d", errno);
                }

                int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);

                if (len < 0)
                {
                    ESP_LOGE(ERRORTAG, "recv failed: errno %d", errno);
                }

                else
                {
                    rx_buffer[len] = '\0';
                    ESP_LOGI(INFOTAG, "Received: %s",rx_buffer);
                    uart_print_str(UART_NUMBER, "\r\n");
                    uart_print_str(UART_NUMBER, rx_buffer);
                    uart_print_str(UART_NUMBER, "\r\n");
                }
                memset(rx_buffer, 0, 128);
                j--;
                i++;

                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            if (sock != -1)
            {
                ESP_LOGI(INFOTAG, "Shutting down socket");
                shutdown(sock, 0);
                close(sock);
            }
        }
    }

    free(socket_params->ip);
    vTaskDelete(NULL);
}

void handle_sock_ping(char* ip, int port, int count)
{
    socket_params_s socket_params[1];

    socket_params->ip = ip, socket_params->port = port,
    socket_params->count = count,

    xQueueSend(socket_params_queue, &socket_params, portMAX_DELAY);

    xTaskCreate(wifi_ping_task, "ping some server", 4096, NULL, 1, NULL);
}

void handle_help()
{
    uart_print_str(UART_NUMBER, "\n\rExamples of commands you may use:");
    uart_print_str(UART_NUMBER, "\n\rconnect -s=AP ssid  -p=password");
    uart_print_str(UART_NUMBER, "\n\rconnect -s AP ssid  -p password");
    uart_print_str(UART_NUMBER, "\n\rconnection-status");
    uart_print_str(UART_NUMBER, "\n\rdisconnect");
    uart_print_str(UART_NUMBER, "\n\rhttp-get -u \"iot-track.vsyveniu.com\"");
}
