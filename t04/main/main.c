/** @file main.c *
 * entry point
 */

#include "main.h"
#include "esp_wifi.h"
#include "uart_console.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_sntp.h"
#include "lwip/apps/sntp.h"
#include "timer_utils.h"
#include "oled.h"
#include <driver/dac.h>

static TaskHandle_t alarm_task_handle = NULL;
xQueueHandle butt_queue;

/* static const char* ERRORTAG = "error: ";
static const char* INFOTAG = "info: "; */

static void IRAM_ATTR butt1_handler(void* args)
{
    uint32_t pin = (uint32_t)args;
    xQueueSendFromISR(butt_queue, &pin, NULL);
}

void butt1_pushed()
{
    uint32_t pin;

    while (true)
    {
        if (xQueueReceive(butt_queue, &pin, portMAX_DELAY))
        {
            if (alarm_task_handle != NULL)
            {
                vTaskDelete(alarm_task_handle);
                alarm_task_handle = NULL;
            }
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void alarm_ring(void* params)
{
    while (true)
    {
        int beep_count = 0;
        int volt = 0;

        for (beep_count = 0; beep_count < 255; beep_count++)
        {
            for (volt = 0; volt < 256; volt += 30)
            {
                dac_output_voltage(DAC_CHANNEL_1, volt);
            }
            ets_delay_us(1);
        }

        for (beep_count = 255; beep_count > 0; beep_count--)
        {
            for (volt = 256; volt > 0; volt -= 30)
            {
                dac_output_voltage(DAC_CHANNEL_1, volt);
            }
            ets_delay_us(1);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void x_task_oled_time()
{
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds_show = 0;
    uint32_t time_val = 0;
    char* str;

    while (true)
    {
        xTaskNotifyWait(0xffffffff, 0, &time_val, portMAX_DELAY);

        if (time_val == 86400)
        {
            hours = 0;
            minutes = 0;
            seconds_show = 0;
            timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
            timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1 * TIMER_SCALE);
        }
        else
        {
            hours = (time_val) / 3600;
            minutes = (time_val - (3600 * hours)) / 60;
            seconds_show = time_val - (3600 * hours) - (minutes * 60);
        }
        str = make_time_str(hours, minutes, seconds_show);
        if (display_str_fat_row_2(str, 0, 8, 3, 2))
        {
            printf("%s\n", "string is too big");
        }
        free(str);
        uint64_t alarm_val = 0;
        xQueuePeek(alarm_queue, &alarm_val, 5);
        if (alarm_val)
        {
            if (time_val == alarm_val)
            {
                if (alarm_task_handle != NULL)
                {
                    vTaskDelete(alarm_task_handle);
                    alarm_task_handle = NULL;
                    xTaskCreate(alarm_ring, "alarm ring", 2048, NULL, 1,
                                &alarm_task_handle);
                }
                else
                {
                    xTaskCreate(alarm_ring, "alarm ring", 2048, NULL, 1,
                                &alarm_task_handle);
                }
            }
        }
    }
}



void app_main()
{
    esp_err_t err;

    gpio_set_direction(EN_OLED, GPIO_MODE_OUTPUT);
    gpio_set_level(EN_OLED, 1);
    gpio_set_direction(EN_AMP, GPIO_MODE_OUTPUT);
    gpio_set_level(EN_AMP, 1);
    dac_output_enable(DAC_CHANNEL_1);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    nvs_flash_init();

    uart_console_init();
    register_cmnd_set();

    gpio_config_t butt_1_conf = {
        .pin_bit_mask = GPIO_SEL_18,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    gpio_config(&butt_1_conf);

    err = wifi_init();
    if (err != ESP_OK)
    {
        printf("%s\n", "WIFI initialization failed");
    }

    wifi_register_events();

    wifi_sta_info_s wifi_sta_info = {
        .wifi_reconnect_count = 0,
        .state = "DISCONNECTED",
        .ssid = "",
        .passwd = "",
        .ssid_str = "",
        .fallback_ssid = "",
        .fallback_passwd = "",
        .channel = 0,
        .rssi = 0,
        .ip = NULL,
        .is_connected = false,
    };

    wifi_get_nvs_data(&wifi_sta_info);

    wifi_info_queue = xQueueCreate(1, sizeof(wifi_sta_info_s));

    UBaseType_t is_filled = 0;
    is_filled = uxQueueMessagesWaiting(wifi_info_queue);

    if (!is_filled)
    {
        xQueueSend(wifi_info_queue, &wifi_sta_info, 10);
    }

    wifi_connect(wifi_sta_info.ssid_str, wifi_sta_info.passwd);

    err = init_oled();
    if (err != ESP_OK)
    {
        printf("%s\n", "couldn't initiate oled");
    }
    clear_oled();

    xTaskCreate(x_task_oled_time, "push time to oled", 4096, NULL, 1,
                &notify_time_change);

    timer_0_init();            

    display_str("Time sync...", 3, 0, 6);

    nvs_open("sntp_store", NVS_READWRITE, &sntp_nvs_handle);

    char* timezone;
    size_t timezone_len = 0;

    esp_err_t is_timezone_saved =
        nvs_get_str(sntp_nvs_handle, "timezone", NULL, &timezone_len);

    if (is_timezone_saved != ESP_OK)
    {
        printf("%s %s\n", "couldn't find key",
               esp_err_to_name(is_timezone_saved));
    }

    if (is_timezone_saved == ESP_OK)
    {
        timezone = (char*)malloc(timezone_len);
        memset(timezone, 0, timezone_len);
        is_timezone_saved =
            nvs_get_str(sntp_nvs_handle, "timezone", timezone, &timezone_len);
        sntp_time_set(timezone);
        if (is_timezone_saved != ESP_OK)
        {
            printf("%s\n", "couldn't get nvs ssid value");
        }
        free(timezone);
    }
    else
    {
        sntp_time_set("UTC-2");
    }

    nvs_close(sntp_nvs_handle);

    timer_start(TIMER_GROUP_0, TIMER_0);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTT_1, butt1_handler, (void*)BUTT_1);

    butt_queue = xQueueCreate(1, sizeof(int));
    alarm_queue = xQueueCreate(1, sizeof(uint64_t));

    xTaskCreate(butt1_pushed, "button 1 task", 2048, NULL, 1, NULL);
}
