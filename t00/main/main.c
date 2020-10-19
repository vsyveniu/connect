/** @file main.c *
* entry point
*/

#include "main.h"
#include "esp_wifi.h"

void app_main()
{
    wifi_init_config_t wifi_config  = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&wifi_config);

    esp_wifi_set_mode(WIFI_MODE_STA);

  /*   wifi_mode_t mode;

    esp_wifi_get_mode(&mode);

    printf("%d\n", mode); */

    
}
