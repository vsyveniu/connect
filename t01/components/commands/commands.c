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


esp_err_t ip_validate(char *ip)
{
    esp_err_t status = ESP_OK;

    for (int i = 0; ip[i] && status == ESP_OK; ++i) {
		if (isdigit(ip[i]) == 0 && ip[i] != '.') {
			status = ESP_FAIL;
		}
	}
	for (int i = 0; ip[i + 1] && status == ESP_OK; ++i) {
		if (ip[i] == '.' && ip[i + 1] == '.') {
			status = ESP_FAIL;
		}
	}
    return (status);
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
    esp_err_t is_valid = ip_validate(ip_copy);

    printf("is_valid %d\n", is_valid);

    printf("errors %d\n", nerrors);
    printf("aton %d\n", aton_result);
    printf("$%s$\n", ip_copy);

    if (nerrors > 0 || is_valid == ESP_FAIL)
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