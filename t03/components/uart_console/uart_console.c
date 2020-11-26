#include "uart_console.h"
#include <ctype.h>

static QueueHandle_t uart0_queue;
xSemaphoreHandle mutexInput;

void uart_event_task(void* pvParams)
{
    uart_event_t event;
    static BaseType_t xHigherPriorityTaskWoken;
    while (true)
    {
        if (xQueueReceive(uart0_queue, (void*)&event,
                          (portTickType)portMAX_DELAY))
        {
            vTaskResume(pvParams);
            xSemaphoreGiveFromISR(mutexInput, &xHigherPriorityTaskWoken);
        }
    }
}

void cmd_instance_task(void* pvParams)
{
    char* prompt = "\r> ";
    uint8_t buff[256];
    memset(buff, 0, 256);
    char str[256];
    memset(str, '\0', 256);
    int i = 0;
    int j = 0;
    int ret = 0;
    int is_first = 1;
    esp_err_t console_ret = 0;

    while (true)
    {
        if (xSemaphoreTake(mutexInput, portMAX_DELAY))
        {
            uart_saved_input_s uart_saved[1];
            xQueuePeek(uart_save_input_queue, &uart_saved, 10);

            if (is_first)
            {
                uart_write_bytes(UART_NUMBER, prompt, strlen(prompt));
                is_first = 0;
            }
            int rxlen;
            ret = 0;
            console_ret = 0;
            rxlen =
                uart_read_bytes(UART_NUM_1, buff, 256, 20 / portTICK_RATE_MS);

            if (buff[0] == 13)
            {
                uart_saved->p_saved = NULL;
                int z = 0;
                while (z < strlen(str) && str[z] != '\0')
                {
                    if (!isprint(str[z]))
                    {
                        str[z] = ' ';
                    }
                    z++;
                }
                xQueueOverwrite(uart_save_input_queue, &uart_saved);
                console_ret = esp_console_run(str, &ret);
                if (console_ret == ESP_ERR_INVALID_ARG || i == 0)
                {
                    uart_print_str(UART_NUMBER,
                                   "\n\n\rWhat am i supposed to do?\n");
                }
                else if (console_ret == ESP_ERR_NOT_FOUND)
                {
                    uart_print_str(UART_NUMBER, "\n\n\rCommand not found\n");
                }
                memset(str, '\0', 256);
                memset(buff, 0, 256);
                i = 0;

                if (ret == 1)
                {
                    is_first = 1;
                    uart_print_str(
                        UART_NUMBER,
                        "\n\rType anything to enter a REPL again\n\r");
                    vTaskSuspend(NULL);
                }
                else
                {
                    uart_print_str(UART_NUMBER, "\n\r");
                    uart_write_bytes(UART_NUMBER, prompt, strlen(prompt));
                }
            }
            else if (rxlen > 0)
            {
                j = 0;
                if (buff[0] == 127 && i > 0)
                {
                    uart_write_bytes(UART_NUMBER, "\b", 1);
                    uart_write_bytes(UART_NUMBER, " ", 1);
                    uart_write_bytes(UART_NUMBER, "\b", 1);
                    i--;
                    str[i] = buff[j];
                    j++;
                }
                if (isprint(buff[0]))
                {
                    j = 0;
                    if (rxlen + strlen(str) > 255 && strlen(str) < 255)
                    {
                        uart_write_bytes(UART_NUMBER, buff, 255 - strlen(str));
                        while (i < 255)
                        {
                            str[i] = buff[j];
                            i++;
                            j++;
                        }
                    }
                    else if (i < 255)
                    {
                        uart_write_bytes(UART_NUMBER, buff, rxlen);
                        while (j < rxlen)
                        {
                            str[i] = buff[j];
                            i++;
                            j++;
                        }
                        uart_saved->p_saved = str;
                        xQueueOverwrite(uart_save_input_queue, &uart_saved);
                    }
                }
            }
            memset(buff, 0, 256);
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

int8_t uart_console_init()
{
    esp_err_t err;

    esp_console_config_t console_conf = {
        .max_cmdline_length = 256,
        .max_cmdline_args = 12,
        .hint_color = 37,
        .hint_bold = 1,
    };

    err = esp_console_init(&console_conf);
    if (err != ESP_OK)
    {
        return err;
    }

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,

    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUMBER, &uart_config));
    uart_set_pin(UART_NUMBER, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);
    ESP_ERROR_CHECK(
        uart_driver_install(UART_NUMBER, 1024, 0, 20, &uart0_queue, 0));

    TaskHandle_t xcmdHandle = NULL;

    mutexInput = xSemaphoreCreateBinary();
    uart_mutex_output = xSemaphoreCreateMutex();

    uart_save_input_queue = xQueueCreate(1, sizeof(uart_saved_input_s));

    uart_enable_tx_intr(UART_NUMBER, 1, 1);

    uart_saved_input_s uart_saved[1];
    uart_saved->p_saved = NULL;
    xQueueSend(uart_save_input_queue, &uart_saved, 10);

    xTaskCreate(cmd_instance_task, "cmd_instance_task", 4096, NULL, 2,
                &xcmdHandle);
    xTaskCreate(uart_event_task, "uart_event_task", 2048, xcmdHandle, 2, NULL);

    uart_print_str(UART_NUMBER, "\n\rType anything to enter a REPL \n\n\r");

    return ESP_OK;
}
