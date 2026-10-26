#include "app_web.h"
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "app_web";

static const char *HTML_PAGE =
    "<html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<style>body{font-family:sans-serif;text-align:center;margin-top:60px;}"
    "a{display:inline-block;padding:20px 40px;margin:10px;font-size:24px;"
    "color:white;border-radius:8px;text-decoration:none;}"
    ".on{background:#27ae60;} .off{background:#e74c3c;}</style></head>"
    "<body><h2>BLE Mesh Switch Control</h2>"
    "<a class=\"on\" href=\"/on\">ON</a>"
    "<a class=\"off\" href=\"/off\">OFF</a>"
    "</body></html>";

static set_switch_f s_set_switch_cb = NULL;
static httpd_handle_t s_server = NULL;

static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t on_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Web request: Turn Switch ON");
    if (s_set_switch_cb) {
        s_set_switch_cb(true);
    }
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t off_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Web request: Turn Switch OFF");
    if (s_set_switch_cb) {
        s_set_switch_cb(false);
    }
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
}

void appweb_init(set_switch_f cb)
{
    s_set_switch_cb = cb;
}

void appweb_start_server(void)
{
    if (s_server != NULL) {
        ESP_LOGW(TAG, "Server already started");
        return;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8;
    config.max_resp_headers = 16;

    ESP_LOGI(TAG, "Starting web server on port: %d", config.server_port);
    if (httpd_start(&s_server, &config) == ESP_OK) {
        httpd_uri_t root_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(s_server, &root_get);

        httpd_uri_t on_get = {
            .uri = "/on",
            .method = HTTP_GET,
            .handler = on_get_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(s_server, &on_get);

        httpd_uri_t off_get = {
            .uri = "/off",
            .method = HTTP_GET,
            .handler = off_get_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(s_server, &off_get);
    } else {
        ESP_LOGE(TAG, "Error starting web server!");
    }
}
