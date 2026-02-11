#include "sd_card.h"

#if SPECTRA_STORAGE_SD_ACTIVE

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include "sdmmc_cmd.h"
#include "soc/spi_pins.h"

static const char *TAG = "sd_card";

#ifndef SPECTRA_SD_SPI_HOST
#define SPECTRA_SD_SPI_HOST SPI2_HOST
#endif

#ifndef SPECTRA_SD_PIN_MOSI
#define SPECTRA_SD_PIN_MOSI GPIO_NUM_23
#endif

#ifndef SPECTRA_SD_PIN_MISO
#define SPECTRA_SD_PIN_MISO GPIO_NUM_24
#endif

#ifndef SPECTRA_SD_PIN_SCLK
#define SPECTRA_SD_PIN_SCLK GPIO_NUM_25
#endif

#ifndef SPECTRA_SD_PIN_CS
#define SPECTRA_SD_PIN_CS GPIO_NUM_26
#endif

static sdmmc_card_t *s_card = NULL;
static bool s_sd_mounted = false;
static bool s_bus_owned = false;
static sd_card_status_t s_status = {
    .mounted = false,
    .total_bytes = 0,
    .free_bytes = 0,
};
typedef struct {
    gpio_num_t mosi;
    gpio_num_t miso;
    gpio_num_t sclk;
    gpio_num_t cs;
    const char *name;
} sd_pin_set_t;

static bool is_mspi_pin(gpio_num_t pin) {
    switch (pin) {
        case MSPI_IOMUX_PIN_NUM_HD:
        case MSPI_IOMUX_PIN_NUM_WP:
        case MSPI_IOMUX_PIN_NUM_CS0:
        case MSPI_IOMUX_PIN_NUM_CLK:
        case MSPI_IOMUX_PIN_NUM_MOSI:
        case MSPI_IOMUX_PIN_NUM_MISO:
        case MSPI_IOMUX_PIN_NUM_CS1:
            return true;
        default:
            return false;
    }
}

static bool pins_conflict_mspi(const sd_pin_set_t *pins) {
    return is_mspi_pin(pins->mosi) || is_mspi_pin(pins->miso) || is_mspi_pin(pins->sclk) || is_mspi_pin(pins->cs);
}

static esp_err_t sd_mount_with_pins(const sd_pin_set_t *pins) {
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = pins->mosi,
        .miso_io_num = pins->miso,
        .sclk_io_num = pins->sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 16 * 1024,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .intr_flags = 0,
    };

    esp_err_t err = spi_bus_initialize(SPECTRA_SD_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (err == ESP_OK) {
        s_bus_owned = true;
    } else if (err == ESP_ERR_INVALID_STATE) {
        // Another module already initialized the bus; continue with shared ownership.
        s_bus_owned = false;
        err = ESP_OK;
    } else {
        return err;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPECTRA_SD_SPI_HOST;
    host.max_freq_khz = SDMMC_FREQ_PROBING;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = pins->cs;
    slot_config.host_id = SPECTRA_SD_SPI_HOST;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false,
    };

    err = esp_vfs_fat_sdspi_mount(SPECTRA_SD_BASE_PATH, &host, &slot_config, &mount_config, &s_card);
    if (err != ESP_OK) {
        if (s_bus_owned) {
            spi_bus_free(SPECTRA_SD_SPI_HOST);
            s_bus_owned = false;
        }
        return err;
    }

    return ESP_OK;
}

static esp_err_t make_sd_path(const char *relative_path, char *out_path, size_t out_path_len) {
    if (relative_path == NULL || out_path == NULL || out_path_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    int written = 0;
    if (relative_path[0] == '/') {
        written = snprintf(out_path, out_path_len, "%s%s", SPECTRA_SD_BASE_PATH, relative_path);
    } else {
        written = snprintf(out_path, out_path_len, "%s/%s", SPECTRA_SD_BASE_PATH, relative_path);
    }

    if (written < 0 || (size_t)written >= out_path_len) {
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

esp_err_t sd_card_get_free_space(uint64_t *out_free_bytes, uint64_t *out_total_bytes) {
    if (!s_sd_mounted || out_free_bytes == NULL || out_total_bytes == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    FATFS *fs = NULL;
    DWORD free_clusters = 0;
    FRESULT fr = f_getfree("0:", &free_clusters, &fs);
    if (fr != FR_OK || fs == NULL) {
        return ESP_FAIL;
    }

    const uint64_t cluster_bytes = (uint64_t)fs->csize * 512ULL;
    *out_total_bytes = (uint64_t)(fs->n_fatent - 2U) * cluster_bytes;
    *out_free_bytes = (uint64_t)free_clusters * cluster_bytes;
    return ESP_OK;
}

esp_err_t sd_card_init(void) {
    if (s_sd_mounted) {
        return ESP_OK;
    }

    const sd_pin_set_t pins = {
        .mosi = SPECTRA_SD_PIN_MOSI,
        .miso = SPECTRA_SD_PIN_MISO,
        .sclk = SPECTRA_SD_PIN_SCLK,
        .cs = SPECTRA_SD_PIN_CS,
        .name = "default",
    };

    // Safety check: reconfiguring MSPI pins can corrupt flash access and hard-lock the CPU.
    if (pins_conflict_mspi(&pins)) {
        ESP_LOGE(TAG,
                 "Invalid SD pin mapping (conflicts with MSPI flash pins): MOSI=%d MISO=%d SCLK=%d CS=%d",
                 (int)pins.mosi,
                 (int)pins.miso,
                 (int)pins.sclk,
                 (int)pins.cs);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = sd_mount_with_pins(&pins);
    if (err != ESP_OK) {
        ESP_LOGW(TAG,
                 "SD mount failed (%s): pins mosi=%d miso=%d sclk=%d cs=%d: %s",
                 pins.name,
                 (int)pins.mosi,
                 (int)pins.miso,
                 (int)pins.sclk,
                 (int)pins.cs,
                 esp_err_to_name(err));
        return err;
    }

    s_sd_mounted = true;
    s_status.mounted = true;
    if (sd_card_get_free_space(&s_status.free_bytes, &s_status.total_bytes) == ESP_OK) {
        ESP_LOGI(TAG,
                 "SD mounted (%s): pins mosi=%d miso=%d sclk=%d cs=%d total=%llu free=%llu",
                 pins.name,
                 (int)pins.mosi,
                 (int)pins.miso,
                 (int)pins.sclk,
                 (int)pins.cs,
                 (unsigned long long)s_status.total_bytes,
                 (unsigned long long)s_status.free_bytes);
    } else {
        ESP_LOGI(TAG,
                 "SD mounted (%s): pins mosi=%d miso=%d sclk=%d cs=%d",
                 pins.name,
                 (int)pins.mosi,
                 (int)pins.miso,
                 (int)pins.sclk,
                 (int)pins.cs);
    }
    return ESP_OK;
}

esp_err_t sd_card_deinit(void) {
    if (!s_sd_mounted) {
        return ESP_OK;
    }

    esp_err_t err = esp_vfs_fat_sdcard_unmount(SPECTRA_SD_BASE_PATH, s_card);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "SD unmount warning: %s", esp_err_to_name(err));
    }

    if (s_bus_owned) {
        esp_err_t bus_err = spi_bus_free(SPECTRA_SD_SPI_HOST);
        if (bus_err != ESP_OK) {
            ESP_LOGW(TAG, "spi_bus_free warning: %s", esp_err_to_name(bus_err));
        }
    }

    s_card = NULL;
    s_sd_mounted = false;
    s_bus_owned = false;
    s_status.mounted = false;
    s_status.total_bytes = 0;
    s_status.free_bytes = 0;
    return ESP_OK;
}

bool sd_card_is_mounted(void) { return s_sd_mounted; }

esp_err_t sd_card_write_file(const char *relative_path, const void *data, size_t len) {
    if (!s_sd_mounted || relative_path == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char path[224];
    ESP_RETURN_ON_ERROR(make_sd_path(relative_path, path, sizeof(path)), TAG, "Invalid SD path");

    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return ESP_FAIL;
    }

    const size_t written = fwrite(data, 1, len, fp);
    fclose(fp);

    if (written != len) {
        ESP_LOGE(TAG, "Incomplete write to %s", path);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t sd_card_read_file(const char *relative_path, void *out_data, size_t max_len, size_t *out_read_len) {
    if (!s_sd_mounted || relative_path == NULL || out_data == NULL || out_read_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char path[224];
    ESP_RETURN_ON_ERROR(make_sd_path(relative_path, path, sizeof(path)), TAG, "Invalid SD path");

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    const size_t read_len = fread(out_data, 1, max_len, fp);
    fclose(fp);
    *out_read_len = read_len;
    return ESP_OK;
}

esp_err_t sd_card_list_dir(const char *relative_dir, size_t *out_entries) {
    if (!s_sd_mounted || relative_dir == NULL || out_entries == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char path[224];
    ESP_RETURN_ON_ERROR(make_sd_path(relative_dir, path, sizeof(path)), TAG, "Invalid SD directory");

    DIR *dir = opendir(path);
    if (dir == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    size_t count = 0;
    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        count++;
    }
    closedir(dir);

    *out_entries = count;
    return ESP_OK;
}

esp_err_t sd_card_get_status(sd_card_status_t *out_status) {
    if (out_status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *out_status = s_status;
    if (s_sd_mounted) {
        (void)sd_card_get_free_space(&out_status->free_bytes, &out_status->total_bytes);
    }
    return ESP_OK;
}

#else

esp_err_t sd_card_init(void) { return ESP_ERR_NOT_SUPPORTED; }

esp_err_t sd_card_deinit(void) { return ESP_OK; }

bool sd_card_is_mounted(void) { return false; }

esp_err_t sd_card_write_file(const char *relative_path, const void *data, size_t len) {
    (void)relative_path;
    (void)data;
    (void)len;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t sd_card_read_file(const char *relative_path, void *out_data, size_t max_len, size_t *out_read_len) {
    (void)relative_path;
    (void)out_data;
    (void)max_len;
    (void)out_read_len;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t sd_card_list_dir(const char *relative_dir, size_t *out_entries) {
    (void)relative_dir;
    (void)out_entries;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t sd_card_get_free_space(uint64_t *out_free_bytes, uint64_t *out_total_bytes) {
    (void)out_free_bytes;
    (void)out_total_bytes;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t sd_card_get_status(sd_card_status_t *out_status) {
    if (out_status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    out_status->mounted = false;
    out_status->total_bytes = 0;
    out_status->free_bytes = 0;
    return ESP_OK;
}

#endif
