#include "socket_connection.h"

static const char* ERRORTAG = "error: ";
static const char* INFOTAG = "info: ";

esp_err_t socket_set_options(int sock)
{
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,
                   sizeof(timeout)) < 0)
    {
        ESP_LOGE(ERRORTAG, "setsockopt failed %d", errno);
    }
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout,
                   sizeof(timeout)) < 0)
    {
        ESP_LOGE(ERRORTAG, "setsockopt failed %d", errno);
    }

    return (ESP_OK);
}

esp_err_t dns_find_by_hostname(char* host, ip_addr_t* ip_Addr)
{
    esp_err_t err;

    err = dns_gethostbyname(host, ip_Addr, NULL, NULL);

    if (err == -16)
    {
        printf("dns err %s\n", "wrong host name");
        uart_clear_up_line();
        uart_print_str(UART_NUMBER, "DNS couldn't find ip by hostname");
        uart_clear_line();
    }
    else if (err < 0)
    {
        int8_t dns_failed_times = 42;
        while (err < 0)
        {
            err = dns_gethostbyname(host, ip_Addr, NULL, NULL);
            printf("dns err %s\n", esp_err_to_name(err));
            if (dns_failed_times == 0)
            {
                return (ESP_FAIL);
            }
            dns_failed_times--;
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    return (ESP_OK);
}

esp_err_t socket_create(int addr_family, int type, int ip_protocol,
                        struct sockaddr_in* dest_addr, int *sock)
{
    *sock = socket(addr_family, type, ip_protocol);

    if (*sock < 0)
    {
        ESP_LOGE(ERRORTAG, "Unable to create socket: errno %d", errno);
        return (ESP_FAIL);
    }

    esp_err_t err;
    err = connect(*sock, (struct sockaddr*)dest_addr, sizeof(*dest_addr));
    if (err != 0)
    {
        ESP_LOGE(ERRORTAG, "Socket unable to connect: errno %d", errno);
        return (ESP_FAIL);
    }
    ESP_LOGI(INFOTAG, "Successfully connected");
    return (ESP_OK);
}