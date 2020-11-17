#include "commands.h"
#include "argtable3/argtable3.h"
#include "timer_utils.h"
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

    esp_console_cmd_t cmd_timezone_conf = {
        .command = "set-timezone",
        .func = &cmd_timezone_set,
    };

    esp_console_cmd_t cmd_set_alarm_conf = {
        .command = "alarm",
        .func = &cmd_alarm,
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
    esp_console_cmd_register(&cmd_timezone_conf);
    esp_console_cmd_register(&cmd_set_alarm_conf);
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

int cmd_alarm(int argc, char** argv)
{
    int nerrors = 0;

    void* argtable[] = {
        alarm_set =
            arg_rex1("val", "value",
                     "(?:[01]\\d|2[0123]):(?:[012345]\\d):(?:[012345]\\d)",
                     "<n>", 0, "the regular expression option"),
        end = arg_end(20),
    };

    nerrors = arg_parse(argc, argv, argtable);
    if (nerrors > 0)
    {
        uart_print_str(UART_NUMBER, "\n\rarguments line error");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    handle_alarm(alarm_set);

    return 0;
}

int cmd_timezone_set(int argc, char** argv)
{
    int8_t nerrors = 0;

    void* argtable[] = {
        timezone = arg_str1("z", NULL, "<string>", "the timezone option"),
        end = arg_end(20),
    };

    nerrors = arg_parse(argc, argv, argtable);

    if (nerrors > 0)
    {
        uart_print_str(UART_NUMBER, "\n\rarguments line error\n\r");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 0;
    }
    char *zone = calloc(strlen(*timezone->sval) + 1, sizeof(char));

    memcpy(zone, *timezone->sval, strlen(*timezone->sval));

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    bool is_found = 0;
    char* zones[28] = {
        "UTC",    "UTC+1", "UTC+2", "UTC+3",  "UTC+4",  "UTC+5",  "UTC+6",
        "UTC+7",  "UTC+8", "UTC+9", "UTC+10", "UTC+11", "UTC+12", "UTC+13",
        "UTC+14", "UTC-1", "UTC-2", "UTC-3",  "UTC-4",  "UTC-5",  "UTC-6",
        "UTC-7",  "UTC-8", "UTC-9", "UTC-10", "UTC-11", "UTC-5",  NULL};

    for (int i = 0; zones[i]; ++i)
    {
        if (!strcmp(zones[i], zone))
        {
            is_found = true;
            break;
        }
    }
    if(is_found)
    {
        sntp_time_set(zone);
    }
    else
    {
        uart_print_str(UART_NUMBER, "\n\rthis zone not supported\n\r");
    }

    free(zone);
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