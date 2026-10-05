idf_component_register(SRCS "app_ble.c" "app_sensor.c" "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES bt nvs_flash driver freertos ble led tsl2561 i2cdev)
