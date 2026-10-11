#ifndef app_sensor_h_
#define app_sensor_h_

#include <stdbool.h>
#include "driver/gpio.h"

/**
 * @file app_sensor.h
 * @brief LDR Photosensitive Module sensor interface.
 *
 * This version uses a 3-pin LDR Module (LM393 comparator-based) instead
 * of the TSL2561 I2C sensor. The DO (Digital Output) pin connects to GPIO21.
 *
 * Hardware:
 *   - LDR Module VCC  -> 3.3V
 *   - LDR Module GND  -> GND
 *   - LDR Module DO   -> GPIO21
 *
 * The onboard blue potentiometer sets the light threshold physically.
 * When it is DARK  -> DO outputs LOW  (0) -> is_dark = true
 * When it is BRIGHT -> DO outputs HIGH (1) -> is_dark = false
 */

#define LDR_DO_PIN      GPIO_NUM_21   /*!< Digital Output pin from LDR module */

typedef void (*light_changed_f)(bool);

void init_hw(light_changed_f cb);
bool is_light_low(void);

#endif /* app_sensor_h_ */
