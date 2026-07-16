#pragma once

#include "esp_err.h"

esp_err_t battery_init(void);
esp_err_t battery_read_voltage(float *voltage_v);

// Unlike a bare ADC divider, the MAX17048 tracks state of charge itself, so the
// percentage is read from the gauge rather than interpolated from voltage.
esp_err_t battery_read_percentage(int *percentage);
