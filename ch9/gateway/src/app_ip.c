#include "app_ip.h"
#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define TAG "app_ip"

#ifndef WIFI_SSID
#define WIFI_SSID "myssid"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "mypassword"
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static connect_wifi_params_t s_params;
static int s_retry_num = 0;
#define MAXIMUM_RETRY 5

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi station started, connecting...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying connection to AP (attempt %d/%d)...", s_retry_num, MAXIMUM_RETRY);
        } else {
            ESP_LOGE(TAG, "Failed to connect to AP after %d attempts", MAXIMUM_RETRY);
            if (s_params.on_failed) {
                s_params.on_failed();
            }
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        if (s_params.on_connected) {
            s_params.on_connected();
        }
    }
}

void appip_connect_wifi(connect_wifi_params_t params)
{
    s_params = params;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    strncpy((char *)wifi_config.sta.ssid, STR(WIFI_SSID), sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, STR(WIFI_PASS), sizeof(wifi_config.sta.password) - 1);

    // If WIFI_SSID contains enclosing quotes from macro stringification, unquote it
    char *ssid_str = (char *)wifi_config.sta.ssid;
    if (ssid_str[0] == '"') {
        size_t len = strlen(ssid_str);
        if (len > 1 && ssid_str[len - 1] == '"') {
            memmove(ssid_str, ssid_str + 1, len - 2);
            ssid_str[len - 2] = '\0';
        }
    }
    char *pass_str = (char *)wifi_config.sta.password;
    if (pass_str[0] == '"') {
        size_t len = strlen(pass_str);
        if (len > 1 && pass_str[len - 1] == '"') {
            memmove(pass_str, pass_str + 1, len - 2);
            pass_str[len - 2] = '\0';
        }
    }

    ESP_LOGI(TAG, "Connecting to SSID: %s", (char *)wifi_config.sta.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}
