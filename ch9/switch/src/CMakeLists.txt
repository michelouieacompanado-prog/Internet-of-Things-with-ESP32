idf_component_register(SRCS "app_ble.c" "app_sw.c" "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES bt nvs_flash driver freertos ble led)
