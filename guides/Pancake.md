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

**The display and the SD card share one SPI bus.** The C5 exposes a single general-purpose SPI host, and both devices are wired to it, differing only by chip select. The card is mounted through `machine.SDCard` (slot 2, which is SPI2 on this chip) exactly as on the Cardputer, so it lives on MicroPython's VFS, and a POSIX bridge exposes it at `/sdcard` for the C modules that use `fopen()`. `picoware/system/storage.py` already accounts for that arrangement.

This works only because of ordering: `machine.SDCard` always calls `spi_bus_initialize()` itself and raises if the bus is already up, and `ViewManager` builds `Storage` before `Draw`. So the card claims the bus first and the display attaches to it (`lcd.c` tolerates `ESP_ERR_INVALID_STATE`). Two consequences worth knowing before changing anything here:

- `machine.SDCard` fixes the bus at `max_transfer_sz = 4000`, so the display pushes the framebuffer in 6-line chunks (3840 bytes). Raising `LCD_SWAP_LINES` breaks the display whenever a card is inserted.
- Do **not** switch to `esp_vfs_fat_sdspi_mount()` to dodge the ordering constraint. It links ESP-IDF's FatFs next to MicroPython's oofatfs, and both export `f_open`/`f_mount`/... with *different* signatures (oofatfs takes a leading `FATFS*`). The linker matches on name alone, so it builds and then corrupts memory on the first SD access.

**There is no ST7796 driver in the ESP-IDF.** The panel speaks the same generic command set as the ST7789 for everything `esp_lcd` drives, so the ST7789 driver is reused and only the ST7796-specific power/gamma registers are written afterwards, in `lcd_send_st7796_tuning()`.

## Touch controls
With no keys, screen areas are mapped to d-pad buttons in `_TOUCH_ZONES` in `picoware/system/input.py`, following the CrowPanel's scheme:

| Area | Button |
|---|---|
| Top-left corner (70x70) | `BACK` |
| Top edge, centered | `UP` |
| Bottom edge, centered | `DOWN` |
| Left edge, middle | `LEFT` |
| Right edge, middle | `RIGHT` |
| Anywhere else | `CENTER` |

The back corner is not decoration: every view exits on `BUTTON_BACK`, and a board with no keys can only produce the buttons mapped here — without it you can open an app but never leave it. (The CrowPanel's map has no back zone, which is why this one is not an exact copy of it.)

If the zones feel wrong in the hand, they are plain pixel rectangles and can be retuned in `_TOUCH_ZONES` without touching any other board.

## Typing
There is no physical keyboard, so text entry (a WiFi password, for example) goes through Picoware's on-screen keyboard. That keyboard is driven by d-pad *buttons* rather than raw touch points: the tap zones above move a cursor around the key grid, and a tap in the middle (`CENTER`) presses the highlighted key.

Because the board has no keys, `onscreen_keyboard` defaults to **on** here — Picoware's default is off, which suits boards like the Cardputer that have real keys. It remains a toggle under **Settings → On-Screen Keyboard**.

> [!NOTE]
> The default only applies on first boot, when the settings file is created. If Picoware already saved `picoware/settings/picoware.json` to the SD card with the keyboard off, that stored value wins — turn it on in Settings.

## Building

### In CI (no local toolchain)
The `Build Pancake` workflow (`.github/workflows/build-pancake.yml`) builds this board and uploads `Picoware-Pancake.bin` as an artifact. Run it from the Actions tab; it also runs on pushes to `pancake-port` that touch the port. The MicroPython and ESP-IDF versions are workflow inputs if you need to try a different pair.

### Locally
You need bash, ESP-IDF, and a MicroPython checkout that ships the `ESP32_GENERIC_C5` board. **The C5 board first appeared in MicroPython v1.27.0**, and v1.28.0 recommends **ESP-IDF v5.5.1** — keep those two in step. On Windows, build under WSL (the esp32 port does not build natively on Windows).

```bash
# ESP-IDF v5.5.1, C5 toolchain only
git clone -b v5.5.1 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
~/esp-idf/install.sh esp32c5

# MicroPython v1.28.0
git clone -b v1.28.0 https://github.com/micropython/micropython.git ~/micropython

export ESP_IDF_DIR=~/esp-idf
export MICROPYTHON_ROOT=~/micropython
export MICROPYTHON_ESP32_PORT=$MICROPYTHON_ROOT/ports/esp32

. "$ESP_IDF_DIR/export.sh"
make -C "$MICROPYTHON_ESP32_PORT" BOARD=ESP32_GENERIC_C5 submodules

bash tools/micropython-pancake.sh
bash tools/micropython-pancake-flash.sh --port /dev/ttyUSB0
```

The build writes `Picoware-Pancake.bin` (plus the bootloader and partition table) to `builds/MicroPython`.

> [!TIP]
> On Windows, build in WSL but **flash from Windows** against the same `.bin` files. Getting the board's serial port into WSL needs `usbipd` and is more trouble than it is worth.

> [!NOTE]
> The C5's bootloader offset is `0x2000`, not `0x0` as on the ESP32-S3. If you flash with your own tool rather than the script, use that offset or the board will not boot.
