#pragma once

#include "driver/gpio.h"

// LCD (ST7796, 320x480 portrait)
// The LCD, SD card, and (on this board) nothing else share a single SPI bus.
// The ESP32-C5 only exposes one general-purpose SPI host, so both the panel and
// the SD card are devices on SPI2_HOST and only differ by chip select.
#define PANCAKE_LCD_HOST SPI2_HOST
#define PANCAKE_LCD_BL_GPIO GPIO_NUM_26
#define PANCAKE_LCD_RST_GPIO GPIO_NUM_2
#define PANCAKE_LCD_DC_GPIO GPIO_NUM_3
#define PANCAKE_LCD_MOSI_GPIO GPIO_NUM_24
#define PANCAKE_LCD_SCLK_GPIO GPIO_NUM_23
#define PANCAKE_LCD_CS_GPIO GPIO_NUM_5

// MISO is only used by the SD card, but it must be declared when the SPI bus is
// initialized. The LCD usually wins the race to initialize the bus, so it has to
// claim MISO too or the SD card can never read a byte back.
#define PANCAKE_LCD_MISO_GPIO GPIO_NUM_4

// Touch + fuel gauge I2C bus
#define PANCAKE_I2C_PORT I2C_NUM_0
#define PANCAKE_I2C_SDA_GPIO GPIO_NUM_9
#define PANCAKE_I2C_SCL_GPIO GPIO_NUM_10
#define PANCAKE_I2C_FREQ_HZ 400000

// Touch (FT6336U capacitive)
#define PANCAKE_TOUCH_I2C_ADDR 0x38
#define PANCAKE_TOUCH_RST_GPIO GPIO_NUM_8

// Battery fuel gauge (MAX17048)
#define PANCAKE_BATTERY_I2C_ADDR 0x36

// SD card (SDSPI, shares the LCD's bus)
#define PANCAKE_SD_HOST PANCAKE_LCD_HOST
#define PANCAKE_SD_CS_GPIO GPIO_NUM_7
#define PANCAKE_SD_MOSI_GPIO PANCAKE_LCD_MOSI_GPIO
#define PANCAKE_SD_SCLK_GPIO PANCAKE_LCD_SCLK_GPIO
#define PANCAKE_SD_MISO_GPIO PANCAKE_LCD_MISO_GPIO

// Addressable status LED
#define PANCAKE_RGB_LED_GPIO GPIO_NUM_27
