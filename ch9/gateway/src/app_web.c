#include "app_web.h"
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "app_web";

static const char *HTML_FORM =
    "<html><form action=\"/\" method=\"post\">"
    "<label for=\"switch_state\">Set switch:</label>"
    "<select id=\"switch_state\" name=\"switch_state\">"
    "<option value=\"ON\">ON</option>"
    "<option value=\"OFF\">OFF</option>"
    "</select>"
    "<input type=\"submit\" value=\"Submit\">"
    "</form></html>";

static set_switch_f s_set_switch_cb = NULL;
static httpd_handle_t s_server = NULL;

static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t root_post_handler(httpd_req_t *req)
{
    char buf[128] = {0};
    int total_len = req->content_len;
    int cur_len = 0;
    int received = 0;

    if (total_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Content too long");
        return ESP_FAIL;
    }

    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len - cur_len);
        if (received <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI(TAG, "Received form data: %s", buf);

    if (strstr(buf, "switch_state=ON") != NULL) {
        ESP_LOGI(TAG, "Web request: Turn Switch ON");
        if (s_set_switch_cb) {
            s_set_switch_cb(true);
        }
    } else if (strstr(buf, "switch_state=OFF") != NULL) {
        ESP_LOGI(TAG, "Web request: Turn Switch OFF");
        if (s_set_switch_cb) {
            s_set_switch_cb(false);
        }
    }

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
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

    ESP_LOGI(TAG, "Starting web server on port: %d", config.server_port);
    if (httpd_start(&s_server, &config) == ESP_OK) {
        httpd_uri_t root_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(s_server, &root_get);

        httpd_uri_t root_post = {
            .uri = "/",
            .method = HTTP_POST,
            .handler = root_post_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(s_server, &root_post);

        httpd_uri_t root_put = {
            .uri = "/",
            .method = HTTP_PUT,
            .handler = root_post_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(s_server, &root_put);
    } else {
        ESP_LOGE(TAG, "Error starting web server!");
    }
}
