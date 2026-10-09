/**
* ============================================================================
* @file app_sensor.c
* @author Member 2 Name (emmerykelsey.mendoza@g.msuiit.edu.ph)
* @brief TSL2561 Ambient Light Sensor Driver & 30-Lux Threshold Monitor
* @note Polls sensor every 1000ms over I2C (SDA: GPIO21, SCL: GPIO22).
* ============================================================================
*/


#include "app_sensor.h"
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "sensor";
static tsl2561_t s_sensor_dev;
static light_changed_f s_callback = NULL;
static bool s_is_low = false;
static bool s_first_read = true;

static void sensor_task(void *pvParameters)
{
    while (1) {
        uint32_t lux = 0;
        esp_err_t res = tsl2561_read_lux(&s_sensor_dev, &lux);
        if (res == ESP_OK) {
            ESP_LOGI(TAG, "Light level: %u lux", (unsigned int)lux);
            bool is_low = (lux < LIGHT_THRESHOLD);
            if (s_first_read || is_low != s_is_low) {
                s_is_low = is_low;
                s_first_read = false;
                ESP_LOGI(TAG, "Threshold state changed: %s (lux: %u)", s_is_low ? "LOW (DARK)" : "HIGH (BRIGHT)", (unsigned int)lux);
                if (s_callback) {
                    s_callback(s_is_low);
                }
            }
        } else {
            ESP_LOGW(TAG, "Failed to read lux from TSL2561: %s", esp_err_to_name(res));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void init_hw(light_changed_f cb)
{
    s_callback = cb;
    ESP_ERROR_CHECK(i2cdev_init());
    ESP_ERROR_CHECK(tsl2561_init_desc(&s_sensor_dev, LIGHT_ADDR, I2C_NUM_0, LIGHT_SDA, LIGHT_SCL));
    ESP_ERROR_CHECK(tsl2561_init(&s_sensor_dev));
    ESP_LOGI(TAG, "TSL2561 initialized on SDA:%d SCL:%d", LIGHT_SDA, LIGHT_SCL);

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Sensor module calibrated and initialized by Member 2");

}

bool is_light_low(void)
{
    return s_is_low;
}
