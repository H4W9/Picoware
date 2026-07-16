## About
The Marauder Pancake is an ESP32-C5 handheld with a 320x480 ST7796 SPI display and an FT6336U capacitive touch panel. It has no keyboard, so Picoware is driven entirely by touch, in the same way as the CrowPanel.

## Hardware
| | |
|---|---|
| MCU | ESP32-C5 (RISC-V, single core, dual-band WiFi) |
| Display | ST7796, 320x480, SPI, portrait |
| Touch | FT6336U capacitive, I2C |
| Storage | microSD (SPI, shares the display's bus) |
| Battery | MAX17048 fuel gauge, I2C |
| Flash | 8 MB |
| PSRAM | Quad, 40 MHz — **required** |

### Pins
| Function | GPIO |
|---|---|
| Display MOSI / SCLK / MISO | 24 / 23 / 4 |
| Display CS / DC / RST / backlight | 5 / 3 / 2 / 26 |
| SD CS | 7 |
| I2C SDA / SCL | 9 / 10 |
| Touch reset | 8 |
| RGB LED | 27 |

## Notes on this port
Three things about this board differ from the other Picoware ESP32 targets, and they explain most of the code in `src/MicroPython/Pancake`:

**PSRAM is not optional.** Picoware's framebuffer is 8 bits per pixel, so at 320x480 it is 150 KB. That does not fit in the C5's internal SRAM, so `lcd.c` allocates it with `MALLOC_CAP_SPIRAM` and `lcd_init()` fails if PSRAM is unavailable. The build script checks `CONFIG_SPIRAM` for this reason.

**The display and the SD card share one SPI bus.** The C5 exposes a single general-purpose SPI host, and both devices are wired to it, differing only by chip select. This rules out MicroPython's `machine.SDCard`, which always calls `spi_bus_initialize()` and raises if the bus is already up. Instead the card is mounted at the ESP-IDF level (`esp_vfs_fat_sdspi_mount`), and both drivers tolerate the bus already being initialized by the other. Consequently the card lives at `/sdcard` over POSIX and is not on the MicroPython VFS — the same arrangement the Cardputer uses, which `picoware/system/storage.py` already accounts for.

**There is no ST7796 driver in the ESP-IDF.** The panel speaks the same generic command set as the ST7789 for everything `esp_lcd` drives, so the ST7789 driver is reused and only the ST7796-specific power/gamma registers are written afterwards, in `lcd_send_st7796_tuning()`.

## Touch controls
With no keys, screen areas are mapped to d-pad buttons in `_TOUCH_ZONES` in `picoware/system/input.py`, following the CrowPanel's scheme:

| Area | Button |
|---|---|
| Top edge, centered | `UP` |
| Bottom edge, centered | `DOWN` |
| Left edge, middle | `LEFT` |
| Right edge, middle | `RIGHT` |
| Anywhere else | `CENTER` |

If the zones feel wrong in the hand, they are plain pixel rectangles and can be retuned in `_TOUCH_ZONES` without touching any other board.

## Building
The build needs bash, an ESP-IDF 5.5+ install with esp32c5 support, and a MicroPython checkout recent enough to ship the `ESP32_GENERIC_C5` board. On Windows, run it under WSL.

```bash
export MICROPYTHON_ROOT=~/pico/micropython
export MICROPYTHON_ESP32_PORT=$MICROPYTHON_ROOT/ports/esp32
export ESP_IDF_DIR=~/.espressif/v5.5.2/esp-idf

bash tools/micropython-pancake.sh
bash tools/micropython-pancake-flash.sh --port /dev/ttyUSB0
```

The build writes `Picoware-Pancake.bin` (plus the bootloader and partition table) to `builds/MicroPython`.

> [!NOTE]
> The C5's bootloader offset is `0x2000`, not `0x0` as on the ESP32-S3. If you flash with your own tool rather than the script, use that offset or the board will not boot.
