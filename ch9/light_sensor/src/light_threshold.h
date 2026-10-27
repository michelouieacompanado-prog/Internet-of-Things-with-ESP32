#ifndef LIGHT_THRESHOLD_H
#define LIGHT_THRESHOLD_H

#include <stdbool.h>
#include <stdint.h>

#define LIGHT_THRESHOLD 30

static inline bool is_low_light(uint32_t lux)
{
    return lux < LIGHT_THRESHOLD;
}

#endif
