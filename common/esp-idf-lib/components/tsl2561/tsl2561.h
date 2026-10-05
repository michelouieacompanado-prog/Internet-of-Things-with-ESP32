#ifndef __TSL2561_H__
#define __TSL2561_H__

#include <stdbool.h>
#include "i2cdev.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TSL2561_I2C_ADDR_GND   0x29
#define TSL2561_I2C_ADDR_FLOAT 0x39
#define TSL2561_I2C_ADDR_VCC   0x49

typedef enum {
    TSL2561_INTEGRATION_13MS  = 0x00,
    TSL2561_INTEGRATION_101MS = 0x01,
    TSL2561_INTEGRATION_402MS = 0x02,
} tsl2561_integration_time_t;

typedef enum {
    TSL2561_GAIN_1X   = 0x00,
    TSL2561_GAIN_16X  = 0x10,
    TSL2561_GAIN_AUTO = 0xFF,
} tsl2561_gain_t;

typedef enum {
    TSL2561_PACKAGE_CS = 0,
    TSL2561_PACKAGE_T_FN_CL,
} tsl2561_package_t;

typedef struct {
    i2c_dev_t i2c_dev;
    tsl2561_gain_t gain;
    tsl2561_integration_time_t integration_time;
    tsl2561_package_t package_type;
} tsl2561_t;

esp_err_t tsl2561_init_desc(tsl2561_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
esp_err_t tsl2561_free_desc(tsl2561_t *dev);
esp_err_t tsl2561_init(tsl2561_t *dev);
esp_err_t tsl2561_set_gain(tsl2561_t *dev, tsl2561_gain_t gain);
esp_err_t tsl2561_set_integration_time(tsl2561_t *dev, tsl2561_integration_time_t itime);
esp_err_t tsl2561_read_lux(tsl2561_t *dev, uint32_t *lux);

#ifdef __cplusplus
}
#endif

#endif /* __TSL2561_H__ */
