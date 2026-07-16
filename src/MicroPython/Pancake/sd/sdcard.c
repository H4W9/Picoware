/*
 * SD card mounting for the Marauder Pancake.
 *
 * The Cardputer mounts its card through machine.SDCard, but that path is not
 * usable here: it calls spi_bus_initialize() unconditionally and raises if the
 * bus is already up. On this board the panel and the card are two chip selects
 * on one SPI bus (the C5 exposes a single general-purpose host), so whichever
 * driver initializes first would lock the other out.
 *
 * Mounting at the ESP-IDF level instead lets the card attach to an already
 * initialized bus. This yields a real POSIX mount at /sdcard, which is what
 * sd_mp.c and storage.c use, so the Cardputer's MicroPython-VFS POSIX bridge is
 * not needed. As on the Cardputer, the card is not exposed through the
 * MicroPython VFS, and storage.py accounts for that.
 */

#include "sdcard.h"

#include "board_config.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "../../log/log_mp.h"

#ifndef PRINT
#define PRINT(...) LOG_MESSAGE(__VA_ARGS__)
#endif

#define SDCARD_MOUNT_POINT "/sdcard"
#define SDCARD_MAX_FREQ_KHZ 20000
#define SDCARD_MAX_OPEN_FILES 5
#define SDCARD_ALLOCATION_UNIT_SIZE (16 * 1024)

static sdmmc_card_t *s_card;
static bool s_mounted;
static bool s_spi_bus_owned;

bool sdcard_is_mounted(void)
{
    return s_mounted;
}

static esp_err_t sdcard_ensure_spi_bus(void)
{
    spi_bus_config_t bus_cfg = {
        .sclk_io_num = PANCAKE_SD_SCLK_GPIO,
        .mosi_io_num = PANCAKE_SD_MOSI_GPIO,
        .miso_io_num = PANCAKE_SD_MISO_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    esp_err_t err = spi_bus_initialize(PANCAKE_SD_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (err == ESP_ERR_INVALID_STATE)
    {
        // The display already brought the bus up and owns it.
        s_spi_bus_owned = false;
        return ESP_OK;
    }

    if (err == ESP_OK)
    {
        s_spi_bus_owned = true;
    }

    return err;
}

esp_err_t sdcard_mount(void)
{
    if (s_mounted)
    {
        return ESP_OK;
    }

    esp_err_t err = sdcard_ensure_spi_bus();
    if (err != ESP_OK)
    {
        PRINT("SD SPI bus init failed: %s", esp_err_to_name(err));
        return err;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = PANCAKE_SD_HOST;
    host.max_freq_khz = SDCARD_MAX_FREQ_KHZ;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PANCAKE_SD_CS_GPIO;
    slot_config.host_id = PANCAKE_SD_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = SDCARD_MAX_OPEN_FILES,
        .allocation_unit_size = SDCARD_ALLOCATION_UNIT_SIZE,
    };

    err = esp_vfs_fat_sdspi_mount(SDCARD_MOUNT_POINT, &host, &slot_config, &mount_config,
                                  &s_card);
    if (err != ESP_OK)
    {
        PRINT("SD mount failed: %s", esp_err_to_name(err));
        s_card = NULL;

        if (s_spi_bus_owned)
        {
            spi_bus_free(PANCAKE_SD_HOST);
            s_spi_bus_owned = false;
        }
        return err;
    }

    s_mounted = true;
    return ESP_OK;
}

void sdcard_unmount(void)
{
    if (!s_mounted)
    {
        return;
    }

    esp_err_t err = esp_vfs_fat_sdcard_unmount(SDCARD_MOUNT_POINT, s_card);
    if (err != ESP_OK)
    {
        PRINT("SD unmount failed: %s", esp_err_to_name(err));
    }

    s_card = NULL;
    s_mounted = false;

    // Only tear the bus down if the display isn't sharing it.
    if (s_spi_bus_owned)
    {
        spi_bus_free(PANCAKE_SD_HOST);
        s_spi_bus_owned = false;
    }
}
