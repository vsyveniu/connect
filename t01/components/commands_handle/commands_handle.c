#include "commands_handle.h"
#include "driver/periph_ctrl.h"
#include "soc/timer_group_struct.h"
#include <sys/time.h>
#include "sntp.h"
#include "argtable3/argtable3.h"

//#include <sys/socket.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"


static const char* ERRORTAG = "error: ";
static const char* INFOTAG = "info: ";

void handle_ssid_set(struct arg_str *ssid, struct arg_str *passwd)
{
    int32_t ssid_len;
    int32_t passwd_len;

    ssid_len = strlen(*ssid->sval);
    passwd_len = strlen(*passwd->sval);

    char *ssid_copy;
    char *passwd_copy;

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

void handle_connection_status()
{
    wifi_display_info();
}

void handle_disconnect()
{
    esp_wifi_disconnect();
}

void wifi_ping_task(void *params)
{
    socket_params_s socket_params[1];

    if (xQueueReceive(socket_params_queue, &socket_params, portMAX_DELAY))
    {
        printf("%s\n", socket_params->ip);
        printf("%d\n", socket_params->port);
        printf("%d\n", socket_params->count);

        char rx_buffer[128];
        char addr_str[128];
        int addr_family;
        int ip_protocol;

        struct sockaddr_in dest_addr = {
        .sin_addr.s_addr = inet_addr(socket_params->ip),
        .sin_family = AF_INET,
        .sin_port = htons(3000),
        //.sin_port = htons(socket_params->port),
   
    };

    inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        esp_err_t err;

        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(ERRORTAG, "Unable to create socket: errno %d", errno);
        }
        ESP_LOGI(INFOTAG, "Socket created, connecting to %s:%d", socket_params->ip, socket_params->port);

        err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(ERRORTAG, "Socket unable to connect: errno %d", errno);
        }
        else
        {
            ESP_LOGI(INFOTAG, "Successfully connected");

            int32_t i = 0;
            int32_t j = socket_params->count;
 
  
            char control[1000];
            struct sockaddr_in sin;

            while(j > 0)
            {   
                char payload[11];
                //char *payload = "PING #1";
                sprintf(payload, "PING #%d", i);

                printf("%s\n", payload);
                err = send(sock, payload, strlen(payload), 0);
                if (err < 0) {
                    ESP_LOGE(ERRORTAG, "Error occurred during sending: errno %d", errno);
                }
                printf("something was sent? %d\n", err);

                int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                printf("something was received? %d\n", len);

                if (len < 0) {
                    ESP_LOGE(ERRORTAG, "recv failed: errno %d", errno);
                }

                else {
                    rx_buffer[len] = 0; 
                    ESP_LOGI(ERRORTAG, "Received %d bytes from %s:", len, addr_str);
                    ESP_LOGI(ERRORTAG, "%s", rx_buffer);
                }
                j--;
                i++;
                printf("%d\n", i);
            }
            if (sock != -1) {
                ESP_LOGI(ERRORTAG, "Shutting down socket and restarting...");
                shutdown(sock, 0);
                close(sock);
            } 
        }
    } 
    
    free(socket_params->ip);
    vTaskDelete(NULL);
}


void handle_sock_ping(char *ip, int port, int count)
{
    socket_params_s socket_params[1];
    
    printf("ip %s\n", ip);
    socket_params->ip = ip,
    socket_params->port = port,
    socket_params->count = count,

    xQueueSend(socket_params_queue, &socket_params, portMAX_DELAY);
    size_t hs;
    hs = heap_caps_get_free_size(0);

    printf("%d\n", hs);

    xTaskCreate(wifi_ping_task, "ping some server", 4096, NULL, 1, NULL);
}

void handle_help()
{
    uart_print_str(UART_NUMBER, "\n\rExamples of commands you may use:");
    uart_print_str(UART_NUMBER, "\n\rconnect -s=AP ssid  -p=password");
    uart_print_str(UART_NUMBER, "\n\rconnect -s AP ssid  -p password");
    uart_print_str(UART_NUMBER, "\n\rconnection-status");
    uart_print_str(UART_NUMBER, "\n\rdisconnect");
}
