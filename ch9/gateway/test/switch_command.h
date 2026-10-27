#ifndef SWITCH_COMMAND_H
#define SWITCH_COMMAND_H

#include <stdbool.h>
#include <stdint.h>

static inline uint8_t switch_command_value(bool onoff)
{
    return onoff ? 1U : 0U;
}

#endif
