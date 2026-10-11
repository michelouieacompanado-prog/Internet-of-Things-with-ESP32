/**
 * @file app_sensor.c
 * @brief LDR Photosensitive Module driver (3-pin, LM393 comparator-based).
 *
 * Replaces the TSL2561 I2C sensor with a simple digital GPIO read from the
 * DO (Digital Output) pin of the LDR sensor module.
 *
 * Wiring:
 *   LDR Module VCC  -> ESP32 3V3
 *   LDR Module GND  -> ESP32 GND
 *   LDR Module DO   -> ESP32 GPIO21
 *
 * Threshold Calibration:
 *   Turn the blue potentiometer on the LDR module with a small screwdriver
 *   to adjust the light/dark switching point to match your room conditions.
 *   - Clockwise   = more sensitive (triggers in brighter light)
 *   - Anticlockwise = less sensitive (only triggers in very dark conditions)
 */

#include "app_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ldr_sensor";

static light_changed_f s_callback = NULL;
static bool s_is_dark = false;
static bool s_first_read = true;

/**
 * @brief Sensor polling task — reads LDR DO pin every 500ms.
 *
 * The DO pin from the LM393 comparator module outputs:
 *   LOW  (0) when it detects DARKNESS  -> we treat this as "light is low" = true
 *   HIGH (1) when it detects BRIGHTNESS -> we treat this as "light is ok"  = false
 *
 * NOTE: Some LDR modules invert this logic. If your relay behaves backwards,
 * swap the logic below: change (level == 0) to (level == 1).
 */
static void sensor_task(void *pvParameters)
{
    while (1) {
        int level = gpio_get_level(LDR_DO_PIN);

        /* LOW (0) = dark, HIGH (1) = bright */
        bool is_dark = (level == 0);

        if (s_first_read || is_dark != s_is_dark) {
            s_is_dark = is_dark;
            s_first_read = false;
            ESP_LOGI(TAG, "LDR state changed: %s (DO pin level = %d)",
                     s_is_dark ? "DARK (relay ON)" : "BRIGHT (relay OFF)", level);
            if (s_callback) {
                s_callback(s_is_dark);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500)); /* Poll every 500ms */
    }
}

/**
 * @brief Initialize the LDR module GPIO and start the polling task.
 *
 * @param cb  Callback function called when the light state changes.
 *            Receives true  when dark  (relay should turn ON).
 *            Receives false when bright (relay should turn OFF).
 */
void init_hw(light_changed_f cb)
{
    s_callback = cb;

    /* Configure GPIO21 as digital input with internal pull-up */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LDR_DO_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,   /* Pull-up so floating reads HIGH */
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "LDR Module initialized on GPIO%d (DO pin)", LDR_DO_PIN);
    ESP_LOGI(TAG, "Adjust the blue potentiometer on the LDR module to set light threshold.");

    xTaskCreate(sensor_task, "ldr_sensor_task", 2048, NULL, 5, NULL);
}

/**
 * @brief Returns the current darkness state of the sensor.
 * @return true  if it is currently dark (relay should be ON)
 * @return false if it is currently bright (relay should be OFF)
 */
bool is_light_low(void)
{
    return s_is_dark;
}
