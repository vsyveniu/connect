#include "commands.h"
#include "argtable3/argtable3.h"
#include "driver/i2s.h"

#include <sys/socket.h>

struct arg_str* ssid = NULL;
struct arg_str* passwd = NULL;
struct arg_end* end = NULL;
struct arg_str* ip = NULL;
struct arg_int* port = NULL;
struct arg_int* count = NULL;
struct arg_str* url = NULL;
struct arg_str* dht_url = NULL;

void register_cmnd_set()
{
    esp_console_cmd_t cmd_ssid_set_conf = {
        .command = "connect",
        .func = &cmd_ssid_set,
    };

    esp_console_cmd_t cmd_connection_status_conf = {
        .command = "connection-status",
        .func = &cmd_connection_status,
    };

    esp_console_cmd_t cmd_disconnect_conf = {
        .command = "disconnect",
        .func = &cmd_disconnect,
    };

    esp_console_cmd_t cmd_sock_ping_conf = {
        .command = "ping",
        .func = &cmd_sock_ping,
    };

    esp_console_cmd_t cmd_http_get_conf = {
        .command = "http_get",
        .func = &cmd_http_get,
    };

    esp_console_cmd_t cmd_send_dht_conf = {
        .command = "send_dht",
        .func = &cmd_send_dht,
    };

    esp_console_cmd_t cmd_stop_dht_conf = {
        .command = "stop_dht",
        .func = &cmd_stop_dht,
    };

    esp_console_cmd_t cmd_help_conf = {
        .command = "help",
        .func = &cmd_help,
    };

    esp_console_cmd_t cmd_exit_conf = {
        .command = "exit",
        .func = &cmd_exit,
    };

    esp_console_cmd_register(&cmd_ssid_set_conf);
    esp_console_cmd_register(&cmd_connection_status_conf);
    esp_console_cmd_register(&cmd_disconnect_conf);
    esp_console_cmd_register(&cmd_help_conf);
    esp_console_cmd_register(&cmd_sock_ping_conf);
    esp_console_cmd_register(&cmd_http_get_conf);
    esp_console_cmd_register(&cmd_send_dht_conf);
    esp_console_cmd_register(&cmd_stop_dht_conf);
    esp_console_cmd_register(&cmd_exit_conf);
}

int cmd_ssid_set(int argc, char** argv)
{
    int8_t nerrors = 0;

    void* argtable[] = {
        ssid = arg_str1("s", NULL, "<string>", "the ssid option"),
        passwd = arg_strn("p", NULL, "<string>", 0, 1, "the password option"),
        end = arg_end(20),
    };

    nerrors = arg_parse(argc, argv, argtable);

    if (nerrors > 0)
    {
        uart_print_str(UART_NUMBER, "\n\rarguments line error\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }

    handle_ssid_set(ssid, passwd);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}


int cmd_http_get(int argc, char** argv)
{
    int8_t nerrors = 0;

    void* argtable[] = {
        url = arg_str1("u", NULL, "<string>", "the url option"),
        end = arg_end(20),
    };

    nerrors = arg_parse(argc, argv, argtable);

    if (nerrors > 0)
    {
        uart_print_str(UART_NUMBER, "\n\rarguments line error\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }

    if(strstr(*url->sval, "http://") != NULL || strstr(*url->sval, "https://") != NULL)
    {
        uart_print_str(UART_NUMBER, "\n\rurl shouldn't containt prefix, your url is fucked\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }

    if(*url->sval[0] == '/')
    {
        uart_print_str(UART_NUMBER, "\n\rdo you want some file, ya boy? I can give you nothing but fuck off\n\r");
    }

    char *host = calloc(strlen(*url->sval) + 1, sizeof(char));

    memcpy(host, *url->sval, strlen(*url->sval));

    char *query = strchr(host, '/');

    if(query != NULL)
    {
        *query = '\0';
        query++;
    }
    else
    {
        query = "";
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    handle_http_get(host, query);
   
    return 0;
}

int cmd_send_dht(int argc, char** argv)
{
    int8_t nerrors = 0;

    void* argtable[] = {
        dht_url = arg_str1("u", NULL, "<string>", "the url option"),
        end = arg_end(20),
    };

    nerrors = arg_parse(argc, argv, argtable);

    if (nerrors > 0)
    {
        uart_print_str(UART_NUMBER, "\n\rarguments line error\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }

    if(strstr(*dht_url->sval, "http://") != NULL || strstr(*dht_url->sval, "https://") != NULL)
    {
        uart_print_str(UART_NUMBER, "\n\rurl shouldn't containt prefix, your url is fucked\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }

    if(*dht_url->sval[0] == '/')
    {
        uart_print_str(UART_NUMBER, "\n\rdo you want some file, ya boy? I can give you nothing but fuck off\n\r");
    }

    char *host = calloc(strlen(*dht_url->sval) + 1, sizeof(char));

    memcpy(host, *dht_url->sval, strlen(*dht_url->sval));

    char *query = strchr(host, '/');

    if(query != NULL)
    {
        *query = '\0';
        query++;
    }
    else
    {
        query = "";
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    handle_send_dht(host, query);
   
    return 0;
}

int cmd_stop_dht(int argc, char** argv)
{
    if (argc > 1)
    {
        uart_print_str(UART_NUMBER, "\n\rType command without options\n\r");

        return 0;
    }

    handle_stop_dht();

    return 0;
}



int cmd_connection_status(int argc, char** argv)
{
    if (argc > 1)
    {
        uart_print_str(UART_NUMBER, "\n\rType command without options\n\r");

        return 0;
    }

    handle_connection_status();

    return 0;
}

int cmd_disconnect(int argc, char** argv)
{
    if (argc > 1)
    {
        uart_print_str(UART_NUMBER, "\n\rType command without options\n\r");

        return 0;
    }

    handle_disconnect();

    return 0;
}

int cmd_sock_ping(int argc, char** argv)
{
    int8_t nerrors = 0;

    void* argtable[] = {
        ip = arg_str1("i", NULL, "<string>", "ip regexp"),
        port = arg_intn("p", NULL, "<n>", 0, 1, "the password option"),
        count = arg_intn("c", NULL, "<n>", 0, 1, "the password option"),
        end = arg_end(20),
    };
    port->count = 0;
    count->count = 0;
    *port->ival = 0;
    *count->ival = 0;

    nerrors = arg_parse(argc, argv, argtable);

    struct in_addr ip_addr[1];

    int aton_result = 0;
    int ip_len = strlen(*ip->sval);
    char *ip_copy = calloc(ip_len + 1, sizeof(char));
    //char ip_copy[16];
    //memset(ip_copy, 0, 16);
    memcpy(ip_copy, *ip->sval, ip_len);

    aton_result = inet_aton(ip_copy, &ip_addr);

    printf("%d\n", nerrors);
    printf("%d\n", aton_result);
    printf("$%s$\n", ip_copy);

    if (nerrors > 0 || aton_result == 0)
    {
        uart_print_str(UART_NUMBER, "\n\rarguments line error\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }
    if(port->count > 0 && *port->ival < 0)
    {
        uart_print_str(UART_NUMBER, "\n\rport cannot be negative\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }
    if(count->count > 0 && *count->ival < 0)
    {
        uart_print_str(UART_NUMBER, "\n\rcount cannot be negative\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }
    int port_copy = 0; 
    port_copy = *port->ival;
    int count_copy = 0; 
    count_copy = *count->ival;

    port->count = 0;
    count->count = 0;
    *port->ival = 0;
    *count->ival = 0;

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    handle_sock_ping(ip_copy, port_copy, count_copy);

    return 0;
}

int cmd_help(int argc, char** argv)
{
    if (argc > 1)
    {
        uart_print_str(UART_NUMBER, "\n\rType command without options\n\r");

        return 0;
    }
    handle_help();

    return 0;
}

int cmd_exit() { return 1; }