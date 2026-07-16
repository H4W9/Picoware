/*
 * Shared I2C bus for the Marauder Pancake.
 *
 * The FT6336 touch controller and the MAX17048 fuel gauge sit on the same two
 * pins, so neither driver may create or delete the bus on its own. Both call
 * pancake_i2c_bus_acquire() and the bus is only torn down once the last user
 * releases it.
 */

#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Returns the shared bus handle, creating it on first call.
    esp_err_t pancake_i2c_bus_acquire(i2c_master_bus_handle_t *out_bus);

    // Drops one reference; deletes the bus when the count reaches zero.
    void pancake_i2c_bus_release(void);

    // Adds a device to the shared bus at the given 7-bit address.
    esp_err_t pancake_i2c_add_device(uint16_t address, i2c_master_dev_handle_t *out_dev);

#ifdef __cplusplus
}
#endif
