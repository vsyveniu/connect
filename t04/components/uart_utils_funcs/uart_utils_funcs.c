#include "uart_utils_funcs.h"
#include <inttypes.h>

void uart_print_str(int uart_num, char *str)
{
    uart_write_bytes(uart_num, str, strlen(str));
}

void uart_clear_up_line()
{
    uart_print_str(UART_NUMBER, "\e[1A\e[K\n\r");
    uart_print_str(UART_NUMBER, "\e[1A\e[K\n\r");
}

void uart_clear_line()
{
    uart_print_str(UART_NUMBER, "\e[0K\n\n\r");
}


void uart_flush_saved_input()
{
    uart_saved_input_s uart_saved[1];
    uart_saved->p_saved = NULL;
    xQueuePeek(uart_save_input_queue, &uart_saved, 10);
    if (uart_saved->p_saved)
    {
        uart_print_str(UART_NUMBER, "> ");
        uart_print_str(UART_NUMBER, uart_saved->p_saved);
    }
}

void uart_print_int8t_value(char *before, int8_t value, char *after)
{
    if (xSemaphoreTake(uart_mutex_output, portMAX_DELAY))
    {
        char *buff;
        int32_t buff_len = 0;
        buff_len         = sizeof(int32_t) + 1;
        (before != NULL) ? buff_len += strlen(before) : 0;
        (after != NULL) ? buff_len += strlen(after) : 0;

        buff = (char *)malloc(buff_len);
        if (buff != NULL)
        {
            memset(buff, 0, buff_len);

            (before != NULL && after == NULL) ?
                sprintf(buff, "%s%d", before, value) :
                0;
            (after != NULL && before == NULL) ?
                sprintf(buff, "%d%s", value, after) :
                0;
            (before != NULL && after != NULL) ?
                sprintf(buff, "%s%d%s", before, value, after) :
                0;
            (before == NULL && after == NULL) ? sprintf(buff, "%d", value) : 0;

            uart_print_str(UART_NUMBER, buff);
            free(buff);
        }
        xSemaphoreGive(uart_mutex_output);
    }
}

void uart_print_int_value(char *before, int value, char *after)
{
    if (xSemaphoreTake(uart_mutex_output, portMAX_DELAY))
    {
        char *buff;
        int32_t buff_len = 0;
        buff_len         = sizeof(int32_t) + 1;
        (before != NULL) ? buff_len += strlen(before) : 0;
        (after != NULL) ? buff_len += strlen(after) : 0;

        buff = (char *)malloc(buff_len);
        if (buff != NULL)
        {
            memset(buff, 0, buff_len);

            (before != NULL && after == NULL) ?
                sprintf(buff, "%s%d", before, value) :
                0;
            (after != NULL && before == NULL) ?
                sprintf(buff, "%d%s", value, after) :
                0;
            (before != NULL && after != NULL) ?
                sprintf(buff, "%s%d%s", before, value, after) :
                0;
            (before == NULL && after == NULL) ? sprintf(buff, "%d", value) : 0;

            uart_print_str(UART_NUMBER, buff);
            free(buff);
        }
        xSemaphoreGive(uart_mutex_output);
    }
}

void uart_print_uint8t_value(char *before, uint8_t value, char *after)
{
    if (xSemaphoreTake(uart_mutex_output, portMAX_DELAY))
    {
        char *buff;
        int32_t buff_len = 0;
        buff_len         = sizeof(uint32_t) + 1;
        (before != NULL) ? buff_len += strlen(before) : 0;
        (after != NULL) ? buff_len += strlen(after) : 0;

        buff = (char *)malloc(buff_len);
        if (buff != NULL)
        {
            memset(buff, 0, buff_len);

            (before != NULL && after == NULL) ?
                sprintf(buff, "%s%d", before, value) :
                0;
            (after != NULL && before == NULL) ?
                sprintf(buff, "%d%s", value, after) :
                0;
            (before != NULL && after != NULL) ?
                sprintf(buff, "%s%d%s", before, value, after) :
                0;
            (before == NULL && after == NULL) ? sprintf(buff, "%d", value) : 0;

            uart_print_str(UART_NUMBER, buff);
            free(buff);
        }
        xSemaphoreGive(uart_mutex_output);
    }
}

void uart_print_str_value(char *before, char *value, char *after)
{
    if (xSemaphoreTake(uart_mutex_output, portMAX_DELAY))
    {
        char *buff;
        int32_t buff_len = 0;
        buff_len         = strlen(value) + 1;
        (before != NULL) ? buff_len += strlen(before) : 0;
        (after != NULL) ? buff_len += strlen(after) : 0;

        buff = (char *)malloc(buff_len);
        if (buff != NULL)
        {
            memset(buff, 0, buff_len);

            (before != NULL && after == NULL) ?
                sprintf(buff, "%s%s", before, value) :
                0;
            (after != NULL && before == NULL) ?
                sprintf(buff, "%s%s", value, after) :
                0;
            (before != NULL && after != NULL) ?
                sprintf(buff, "%s%s%s", before, value, after) :
                0;
            (before == NULL && after == NULL) ? sprintf(buff, "%s", value) : 0;

            uart_print_str(UART_NUMBER, buff);
            free(buff);
        }
        xSemaphoreGive(uart_mutex_output);
    }
}
