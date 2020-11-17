#include "timer_utils.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "lwip/apps/sntp.h"
#include "oled.h"

static const char* INFOTAG = "info: ";
static const char* ERRORTAG = "error: ";

char *make_time_str(uint8_t hours, uint8_t minutes, uint8_t seconds){
    char *str;
    char seconds_str[5];
    char minutes_str[5];
    char hours_str[5];
    memset(seconds_str, 0, 5);
    memset(minutes_str, 0, 5);
    memset(hours_str, 0, 5);

    str = (char*)calloc(15, sizeof(char));
    if(!str)
    {
      return 0;
    }

    if(seconds < 10)
    {
        sprintf(seconds_str, "0%u", seconds);
    }
    else
    {
        sprintf(seconds_str, "%u", seconds);
    }
    if(minutes < 10)
    {
        sprintf(minutes_str, "0%u", minutes);
    }
    else
    {
        sprintf(minutes_str, "%u", minutes);
    }
    if(hours < 10)
    {
        sprintf(hours_str, "0%u", hours);
    }
    else
    {
        sprintf(hours_str, "%u", hours);
    }

    sprintf(str, "%s:%s:%s", hours_str, minutes_str, seconds_str);

    return (str);
}

void time_sync_notification_cb(struct timeval *tv)
{

uint64_t timer_val = 0;
timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &timer_val);
struct tm timeinfo = { 0 };

localtime_r(&tv->tv_sec, &timeinfo);

timer_val = ((timeinfo.tm_hour * 3600) + (timeinfo.tm_min * 60) + timeinfo.tm_sec);

timer_val *= 1000000;

timer_set_counter_value(TIMER_GROUP_0, TIMER_0, timer_val);
timer_set_alarm_value(TIMER_GROUP_0, TIMER_0,  timer_val + ( 1 * TIMER_SCALE));
ESP_LOGI(INFOTAG, "Time synchronized");
}

esp_err_t sntp_time_set(char *timezone)
{

    sntp_stop();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");

    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    esp_err_t err;
    err = nvs_open("sntp_store", NVS_READWRITE, &sntp_nvs_handle);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't open nvs", esp_err_to_name(err));
    }
    
    err = nvs_set_str(sntp_nvs_handle, "timezone", timezone);
    if (err != ESP_OK)
    {
        printf("%s %s\n", "couldn't store nvs key", esp_err_to_name(err));
    }
    err = nvs_commit(sntp_nvs_handle);
    if (err != ESP_OK)
    {
        printf("%s", esp_err_to_name(err));
    }
    nvs_close(sntp_nvs_handle);

    sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 42;
    setenv("TZ", timezone, 1);
    tzset();

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(INFOTAG, "Waiting for synchronization (%d/%d)", retry, retry_count);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    if(retry == retry_count)
    {
        ESP_LOGI(ERRORTAG, "Can't sync (%d/%d)", retry, retry_count);
        display_str("Couldn't sync :(", 3, 0, 6);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    return (ESP_OK);
}


void IRAM_ATTR timer_intr_handle(void* param)
{
    timer_spinlock_take(TIMER_GROUP_0);
    uint64_t timer_val =
        timer_group_get_counter_value_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, 0);
    uint64_t next_alarm = timer_val + (1 * TIMER_SCALE);
    timer_group_set_alarm_value_in_isr(TIMER_GROUP_0, TIMER_0, next_alarm);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, 0);
    xTaskNotifyFromISR(notify_time_change, timer_val / 1000000,
                       eSetValueWithOverwrite, 0);
    timer_spinlock_give(TIMER_GROUP_0);
}

esp_err_t timer_0_init()
{
    esp_err_t err = 0;
    
      timer_config_t clock_timer_conf = {
        .counter_en = false,
        .counter_dir = TIMER_COUNT_UP,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = false,
        .divider = TIMER_DIVIDER,
    };

    err = timer_init(TIMER_GROUP_0, 0, &clock_timer_conf);
    if (err != ESP_OK)
    {
        printf("%s\n", "couldn't initiate timer");
    }

    err = timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1 * TIMER_SCALE);
    if (err != ESP_OK)
    {
        printf("%s\n", "couldn't set timer alarm");
    }

    err = timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    if (err != ESP_OK)
    {
        printf("%s\n", "couldn't enable timer interrupt");
    }

    err = timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_intr_handle,
                             (void*)0, ESP_INTR_FLAG_IRAM, NULL);
    if (err != ESP_OK)
    {
        printf("%s\n", "couldn't initiate timers interrupt");
    }

    return (ESP_OK);
}