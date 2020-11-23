#include "http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_spiffs.h"
#include "wifi_connection.h"

esp_err_t get_handler(httpd_req_t* req)
{

    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_vfs_spiffs_register(&config);

    char path[600];

    if (strcmp(req->uri, "/") == 0)
    {
        strcpy(path, "/spiffs/index.html");
    }
    else
    { 
        sprintf(path, "/spiffs%s", req->uri);
    }

    char *ptr = strrchr(path, '.');
    if(strcmp(ptr,".css") == 0)
    {
        httpd_resp_set_type(req, "text/css");
    }
   
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        httpd_resp_send_404(req);
    }
    else
    {
        UBaseType_t is_filled = 0;
        char lineRead[256];

        is_filled = uxQueueMessagesWaiting(wifi_scan_queue);
        
        if (is_filled)
        {
            char ret[1048];

            xQueuePeek(wifi_scan_queue, &ret, 10);
            while (fgets(lineRead, sizeof(lineRead), file))
            {
                if(strstr( lineRead, "@scanlist@"))
                {
        
                    char *p_content_continue = strrchr(lineRead, '<');
                    char *p_content_start = strtok(lineRead, "@");

                    httpd_resp_sendstr_chunk(req, p_content_start);
                    httpd_resp_sendstr_chunk(req, ret);
                    httpd_resp_sendstr_chunk(req, p_content_continue);
                   
                }
                else
                {
                    httpd_resp_sendstr_chunk(req,lineRead);
                }
            }
            httpd_resp_sendstr_chunk(req, NULL);
        }
        else
        {
            while (fgets(lineRead, sizeof(lineRead), file))
            {
                httpd_resp_sendstr_chunk(req,lineRead);
            }
            httpd_resp_sendstr_chunk(req, NULL);
        }
    }

    esp_vfs_spiffs_unregister(NULL);

    return ESP_OK;
}


esp_err_t get_asset_handler(httpd_req_t* req)
{

    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_vfs_spiffs_register(&config);

    char path[600];

    if (strcmp(req->uri, "/") == 0)
    {
        strcpy(path, "/spiffs/index.html");
    }
    else
    { 
        sprintf(path, "/spiffs%s", req->uri);
    }

    char *ptr = strrchr(path, '.');
    if(strcmp(ptr,".css") == 0)
    {
        httpd_resp_set_type(req, "text/css");
    }
    if(strcmp(ptr,".gif") == 0)
    {
        httpd_resp_set_type(req, "image/gif");
    }
    if(strcmp(ptr,".png") == 0)
    {
        httpd_resp_set_type(req, "image/png");
    }
   
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        httpd_resp_send_404(req);
    }
    else
    {
        char lineRead[256];
        while (fgets(lineRead, sizeof(lineRead), file))
        {
            httpd_resp_sendstr_chunk(req,lineRead);
        }
        httpd_resp_sendstr_chunk(req, NULL);
    }

    esp_vfs_spiffs_unregister(NULL);

    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t* req)
{
    char content[100];
    char *resp;

    size_t recv_size = sizeof(content);

    int ret = httpd_req_recv(req, content, recv_size);

    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }

        return ESP_FAIL;
    }
    if(strstr(content, "scan"))
    {
        wifi_scan_aps();
        UBaseType_t is_filled = 0;

        is_filled = uxQueueMessagesWaiting(wifi_scan_queue);
        
        if (is_filled)
        {
            char ret[1048];

            xQueuePeek(wifi_scan_queue, &ret, 10);
           
            httpd_resp_send(req, ret, HTTPD_RESP_USE_STRLEN);
        }
    }
    else
    {
        resp = "Can't scan";
    }
    
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL
    };

httpd_uri_t uri_get_css = {
    .uri = "/app.css",
    .method = HTTP_GET,
    .handler = get_asset_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get_bulma = {
    .uri = "/bulma.min.css",
    .method = HTTP_GET,
    .handler = get_asset_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get_js = {
    .uri = "/app.js",
    .method = HTTP_GET,
    .handler = get_asset_handler,
    .user_ctx = NULL
};    

httpd_uri_t uri_post = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL
    };


httpd_handle_t http_server_init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_get_css);
        httpd_register_uri_handler(server, &uri_get_js);
        httpd_register_uri_handler(server, &uri_get_bulma);
        httpd_register_uri_handler(server, &uri_post);
    }

    return server;
}