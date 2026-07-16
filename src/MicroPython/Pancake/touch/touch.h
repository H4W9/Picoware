/*
 * Touch Driver for the Marauder Pancake (FT6336U capacitive, I2C).
 *
 * The FT6336 is read directly over I2C rather than through esp_lcd_touch, which
 * keeps the port from depending on a managed component and matches the register
 * usage this panel is already known to work with.
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define TOUCH_WIDTH 320  // Horizontal resolution
#define TOUCH_HEIGHT 480 // Vertical resolution

#define TOUCH_GPIO_RST 8 // Reset pin
#define TOUCH_GPIO_INT -1 // Interrupt pin (not wired on this board; touch is polled)

// Raising the threshold suppresses phantom touches from a panel mounted in a
// case. Register default is 22; 40 is the value this hardware is tuned to.
#define TOUCH_THRESHOLD 40

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint16_t x;          // X coordinate of the touch point
        uint16_t y;          // Y coordinate of the touch point
        uint16_t strength;   // Touch strength (pressure level)
        uint8_t touch_count; // Number of touch points detected (for multi-touch support)
        bool pressed;        // Whether the touch panel is currently being pressed
    } TouchPoint;

    TouchPoint touch_get_point(void); // get the latest touch coordinates and press state
    void touch_deinit(void);          // deinitialize the touch panel (free resources)
    bool touch_init(void);            // initialize the FT6336 touch panel
    bool touch_read(void);            // read the touch panel data and update coordinates

#ifdef __cplusplus
}
#endif
