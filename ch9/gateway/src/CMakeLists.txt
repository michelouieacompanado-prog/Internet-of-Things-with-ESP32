idf_component_register(SRCS "app_ble.c" "app_ip.c" "app_web.c" "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES bt nvs_flash esp_wifi esp_http_server esp_event esp_netif ble led)
