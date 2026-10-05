#include "i2cdev.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "i2cdev";

typedef struct {
    SemaphoreHandle_t lock;
    bool installed;
} i2c_port_state_t;

static i2c_port_state_t states[I2C_NUM_MAX];

esp_err_t i2cdev_init(void)
{
    for (int i = 0; i < I2C_NUM_MAX; i++) {
        if (!states[i].lock) {
            states[i].lock = xSemaphoreCreateMutex();
            if (!states[i].lock) {
                ESP_LOGE(TAG, "Could not create port mutex");
                return ESP_FAIL;
            }
            states[i].installed = false;
        }
    }
    return ESP_OK;
}

esp_err_t i2cdev_done(void)
{
    for (int i = 0; i < I2C_NUM_MAX; i++) {
        if (states[i].lock) {
            if (states[i].installed) {
                i2c_driver_delete(i);
                states[i].installed = false;
            }
            vSemaphoreDelete(states[i].lock);
            states[i].lock = NULL;
        }
    }
    return ESP_OK;
}

esp_err_t i2c_dev_create_mutex(i2c_dev_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    dev->mutex = xSemaphoreCreateMutex();
    if (!dev->mutex) {
        ESP_LOGE(TAG, "Could not create device mutex");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t i2c_dev_delete_mutex(i2c_dev_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    if (dev->mutex) {
        vSemaphoreDelete(dev->mutex);
        dev->mutex = NULL;
    }
    return ESP_OK;
}

esp_err_t i2c_dev_take_mutex(i2c_dev_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    if (!dev->mutex) return ESP_OK;
    return xSemaphoreTake(dev->mutex, pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT)) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t i2c_dev_give_mutex(i2c_dev_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    if (!dev->mutex) return ESP_OK;
    return xSemaphoreGive(dev->mutex) == pdTRUE ? ESP_OK : ESP_FAIL;
}

static esp_err_t i2c_setup_port(const i2c_dev_t *dev)
{
    if (dev->port >= I2C_NUM_MAX) return ESP_ERR_INVALID_ARG;

    if (!states[dev->port].installed) {
        i2c_config_t conf = dev->cfg;
        conf.mode = I2C_MODE_MASTER;
        esp_err_t res = i2c_param_config(dev->port, &conf);
        if (res != ESP_OK) return res;
        res = i2c_driver_install(dev->port, conf.mode, 0, 0, 0);
        if (res != ESP_OK) return res;
        states[dev->port].installed = true;
    }
    return ESP_OK;
}

esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size)
{
    if (!dev || !in_data || !in_size) return ESP_ERR_INVALID_ARG;

    esp_err_t res = i2c_setup_port(dev);
    if (res != ESP_OK) return res;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (out_data && out_size) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, (void *)out_data, out_size, true);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_READ, true);
    if (in_size > 1) {
        i2c_master_read(cmd, in_data, in_size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, ((uint8_t *)in_data) + in_size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    res = i2c_master_cmd_begin(dev->port, cmd, pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT));
    i2c_cmd_link_delete(cmd);

    return res;
}

esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size)
{
    if (!dev || (!out_data && !out_reg)) return ESP_ERR_INVALID_ARG;

    esp_err_t res = i2c_setup_port(dev);
    if (res != ESP_OK) return res;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    if (out_reg && out_reg_size) {
        i2c_master_write(cmd, (void *)out_reg, out_reg_size, true);
    }
    if (out_data && out_size) {
        i2c_master_write(cmd, (void *)out_data, out_size, true);
    }
    i2c_master_stop(cmd);

    res = i2c_master_cmd_begin(dev->port, cmd, pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT));
    i2c_cmd_link_delete(cmd);

    return res;
}

esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg, void *in_data, size_t in_size)
{
    return i2c_dev_read(dev, &reg, 1, in_data, in_size);
}

esp_err_t i2c_dev_write_reg(const i2c_dev_t *dev, uint8_t reg, const void *out_data, size_t out_size)
{
    return i2c_dev_write(dev, &reg, 1, out_data, out_size);
}
