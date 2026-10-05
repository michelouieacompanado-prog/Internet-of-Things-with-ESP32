#include "app_ledattn.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "led_attn";

void appled_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << ATTN_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(ATTN_LED_PIN, 0);
    ESP_LOGI(TAG, "LED attention initialized on GPIO %d", ATTN_LED_PIN);
}

void appled_set(bool state)
{
    gpio_set_level(ATTN_LED_PIN, state ? 1 : 0);
    ESP_LOGD(TAG, "LED attention set to %s", state ? "ON" : "OFF");
}
