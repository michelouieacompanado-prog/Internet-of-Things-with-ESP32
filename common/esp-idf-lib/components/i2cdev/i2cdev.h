#ifndef __I2CDEV_H__
#define __I2CDEV_H__

#include <driver/i2c.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_I2CDEV_TIMEOUT
#define CONFIG_I2CDEV_TIMEOUT 100000
#endif

#define I2C_DEV_TAKE_MUTEX(dev) do { \
        esp_err_t __err = i2c_dev_take_mutex(dev); \
        if (__err != ESP_OK) return __err; \
    } while (0)

#define I2C_DEV_GIVE_MUTEX(dev) do { \
        esp_err_t __err = i2c_dev_give_mutex(dev); \
        if (__err != ESP_OK) return __err; \
    } while (0)

#define I2CDEV_MAX_PORTS I2C_NUM_MAX

typedef struct {
    i2c_port_t port;
    i2c_config_t cfg;
    uint8_t addr;
    SemaphoreHandle_t mutex;
    uint32_t timeout_ticks;
} i2c_dev_t;

esp_err_t i2cdev_init(void);
esp_err_t i2cdev_done(void);

esp_err_t i2c_dev_create_mutex(i2c_dev_t *dev);
esp_err_t i2c_dev_delete_mutex(i2c_dev_t *dev);
esp_err_t i2c_dev_take_mutex(i2c_dev_t *dev);
esp_err_t i2c_dev_give_mutex(i2c_dev_t *dev);

esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size);
esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size);
esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg, void *in_data, size_t in_size);
esp_err_t i2c_dev_write_reg(const i2c_dev_t *dev, uint8_t reg, const void *out_data, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif /* __I2CDEV_H__ */
