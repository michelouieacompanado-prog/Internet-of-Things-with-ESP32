/**
* ============================================================================
* @file app_sw.c
* @author Member 3 Name (shyanenovelle.canayan@g.msuiit.edu.ph)
* @brief Relay Actuator Hardware Controller (GPIO4)
* @note Responds to Generic OnOff Server (Gateway) and Client (Sensor) events.
* ============================================================================
*/

#include "app_sw.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "switch_hw";
static bool s_relay_state = false;

void init_hw(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(RELAY_PIN, 0);
    s_relay_state = false;
    ESP_LOGI(TAG, "Relay initialized on GPIO %d (State: OFF)", RELAY_PIN);
}

void switch_set(bool state)
{
    s_relay_state = state;
    gpio_set_level(RELAY_PIN, state ? 1 : 0);
    ESP_LOGI(TAG, "Relay state changed to %s", state ? "ON" : "OFF");
}

bool switch_get(void)
{
    return s_relay_state;
}
