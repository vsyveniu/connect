#include "commands.h"
#include "argtable3/argtable3.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include "lwip/apps/sntp.h"

struct arg_str* ssid = NULL;
struct arg_str* passwd = NULL;
struct arg_end* end = NULL;
struct arg_str* ip = NULL;
struct arg_str* timezone = NULL;
struct arg_rex* alarm_set = NULL;
struct arg_str* url = NULL;

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

    esp_console_cmd_t cmd_http_get_conf = {
        .command = "http_get",
        .func = &cmd_http_get,
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
    esp_console_cmd_register(&cmd_http_get_conf);
    esp_console_cmd_register(&cmd_help_conf);
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