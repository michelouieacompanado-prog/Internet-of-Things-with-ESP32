#include "tsl2561.h"
#include "esp_log.h"

#define TSL2561_COMMAND_BIT      (0x80)
#define TSL2561_WORD_BIT         (0x20)
#define TSL2561_CONTROL_POWERON  (0x03)
#define TSL2561_CONTROL_POWEROFF (0x00)

#define TSL2561_REGISTER_CONTROL          (0x00)
#define TSL2561_REGISTER_TIMING           (0x01)
#define TSL2561_REGISTER_THRESHHOLDL_LOW  (0x02)
#define TSL2561_REGISTER_THRESHHOLDL_HIGH (0x03)
#define TSL2561_REGISTER_THRESHHOLDH_LOW  (0x04)
#define TSL2561_REGISTER_THRESHHOLDH_HIGH (0x05)
#define TSL2561_REGISTER_INTERRUPT        (0x06)
#define TSL2561_REGISTER_ID               (0x0A)
#define TSL2561_REGISTER_CHAN0_LOW        (0x0C)
#define TSL2561_REGISTER_CHAN0_HIGH       (0x0D)
#define TSL2561_REGISTER_CHAN1_LOW        (0x0E)
#define TSL2561_REGISTER_CHAN1_HIGH       (0x0F)

#define TSL2561_LUX_LUXSCALE      (14)
#define TSL2561_LUX_RATIOSCALE    (9)
#define TSL2561_LUX_CHSCALE       (10)
#define TSL2561_LUX_CHSCALE_TINT0 (0x7517)
#define TSL2561_LUX_CHSCALE_TINT1 (0x0FE7)

#define TSL2561_LUX_K1T (0x0040)
#define TSL2561_LUX_B1T (0x01f2)
#define TSL2561_LUX_M1T (0x01be)
#define TSL2561_LUX_K2T (0x0080)
#define TSL2561_LUX_B2T (0x0214)
#define TSL2561_LUX_M2T (0x02d1)
#define TSL2561_LUX_K3T (0x00c0)
#define TSL2561_LUX_B3T (0x023f)
#define TSL2561_LUX_M3T (0x037b)
#define TSL2561_LUX_K4T (0x0100)
#define TSL2561_LUX_B4T (0x0270)
#define TSL2561_LUX_M4T (0x03fe)
#define TSL2561_LUX_K5T (0x0138)
#define TSL2561_LUX_B5T (0x016f)
#define TSL2561_LUX_M5T (0x01fc)
#define TSL2561_LUX_K6T (0x019a)
#define TSL2561_LUX_B6T (0x00d2)
#define TSL2561_LUX_M6T (0x00fb)
#define TSL2561_LUX_K7T (0x029a)
#define TSL2561_LUX_B7T (0x0018)
#define TSL2561_LUX_M7T (0x0012)
#define TSL2561_LUX_K8T (0x029a)
#define TSL2561_LUX_B8T (0x0000)
#define TSL2561_LUX_M8T (0x0000)

static const char *TAG = "tsl2561";

esp_err_t tsl2561_init_desc(tsl2561_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    dev->i2c_dev.port = port;
    dev->i2c_dev.addr = addr;
    dev->i2c_dev.cfg.sda_io_num = sda_gpio;
    dev->i2c_dev.cfg.scl_io_num = scl_gpio;
    dev->i2c_dev.cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    dev->i2c_dev.cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    dev->i2c_dev.cfg.master.clk_speed = 100000;
    dev->gain = TSL2561_GAIN_1X;
    dev->integration_time = TSL2561_INTEGRATION_402MS;
    dev->package_type = TSL2561_PACKAGE_T_FN_CL;
    return i2c_dev_create_mutex(&dev->i2c_dev);
}

esp_err_t tsl2561_free_desc(tsl2561_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    return i2c_dev_delete_mutex(&dev->i2c_dev);
}

static esp_err_t tsl2561_enable(tsl2561_t *dev)
{
    return i2c_dev_write_reg(&dev->i2c_dev, TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL,
                             &(uint8_t){TSL2561_CONTROL_POWERON}, 1);
}

static esp_err_t tsl2561_write_timing(tsl2561_t *dev)
{
    uint8_t val = (uint8_t)dev->integration_time | (uint8_t)(dev->gain == TSL2561_GAIN_16X ? 0x10 : 0x00);
    return i2c_dev_write_reg(&dev->i2c_dev, TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, &val, 1);
}

esp_err_t tsl2561_init(tsl2561_t *dev)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    I2C_DEV_TAKE_MUTEX(&dev->i2c_dev);
    esp_err_t res = tsl2561_enable(dev);
    if (res != ESP_OK) {
        I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);
        ESP_LOGE(TAG, "Failed to power on TSL2561: %s", esp_err_to_name(res));
        return res;
    }
    res = tsl2561_write_timing(dev);
    I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);
    return res;
}

esp_err_t tsl2561_set_gain(tsl2561_t *dev, tsl2561_gain_t gain)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    dev->gain = gain;
    I2C_DEV_TAKE_MUTEX(&dev->i2c_dev);
    esp_err_t res = tsl2561_write_timing(dev);
    I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);
    return res;
}

esp_err_t tsl2561_set_integration_time(tsl2561_t *dev, tsl2561_integration_time_t itime)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    dev->integration_time = itime;
    I2C_DEV_TAKE_MUTEX(&dev->i2c_dev);
    esp_err_t res = tsl2561_write_timing(dev);
    I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);
    return res;
}

static uint32_t calculate_lux(uint16_t ch0, uint16_t ch1, tsl2561_gain_t gain, tsl2561_integration_time_t itime)
{
    uint32_t chScale;
    uint32_t channel1;
    uint32_t channel0;

    switch (itime) {
    case TSL2561_INTEGRATION_13MS:
        chScale = TSL2561_LUX_CHSCALE_TINT0;
        break;
    case TSL2561_INTEGRATION_101MS:
        chScale = TSL2561_LUX_CHSCALE_TINT1;
        break;
    default:
        chScale = (1 << TSL2561_LUX_CHSCALE);
        break;
    }

    if (gain != TSL2561_GAIN_16X) {
        chScale = chScale << 4;
    }

    channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
    channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;

    uint32_t ratio1 = 0;
    if (channel0 != 0) {
        ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE + 1)) / channel0;
    }
    uint32_t ratio = (ratio1 + 1) >> 1;

    uint32_t b = 0, m = 0;
    if (ratio <= TSL2561_LUX_K1T) {
        b = TSL2561_LUX_B1T; m = TSL2561_LUX_M1T;
    } else if (ratio <= TSL2561_LUX_K2T) {
        b = TSL2561_LUX_B2T; m = TSL2561_LUX_M2T;
    } else if (ratio <= TSL2561_LUX_K3T) {
        b = TSL2561_LUX_B3T; m = TSL2561_LUX_M3T;
    } else if (ratio <= TSL2561_LUX_K4T) {
        b = TSL2561_LUX_B4T; m = TSL2561_LUX_M4T;
    } else if (ratio <= TSL2561_LUX_K5T) {
        b = TSL2561_LUX_B5T; m = TSL2561_LUX_M5T;
    } else if (ratio <= TSL2561_LUX_K6T) {
        b = TSL2561_LUX_B6T; m = TSL2561_LUX_M6T;
    } else if (ratio <= TSL2561_LUX_K7T) {
        b = TSL2561_LUX_B7T; m = TSL2561_LUX_M7T;
    } else {
        b = TSL2561_LUX_B8T; m = TSL2561_LUX_M8T;
    }

    int32_t temp = ((channel0 * b) - (channel1 * m));
    if (temp < 0) temp = 0;
    temp += (1 << (TSL2561_LUX_LUXSCALE - 1));
    return (uint32_t)(temp >> TSL2561_LUX_LUXSCALE);
}

esp_err_t tsl2561_read_lux(tsl2561_t *dev, uint32_t *lux)
{
    if (!dev || !lux) return ESP_ERR_INVALID_ARG;

    I2C_DEV_TAKE_MUTEX(&dev->i2c_dev);
    uint8_t data[4];
    uint8_t reg = TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW;
    esp_err_t res = i2c_dev_read(&dev->i2c_dev, &reg, 1, data, 4);
    I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);

    if (res != ESP_OK) {
        return res;
    }

    uint16_t ch0 = ((uint16_t)data[1] << 8) | data[0];
    uint16_t ch1 = ((uint16_t)data[3] << 8) | data[2];

    *lux = calculate_lux(ch0, ch1, dev->gain, dev->integration_time);
    return ESP_OK;
}
