#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "esp_err.h"
#include "sdkconfig.h"
#include "soc/soc_caps.h"

#define SPECTRA_SD_BASE_PATH "/sdcard"

#ifndef CONFIG_SPECTRA_STORAGE_ENABLE_SD
#define CONFIG_SPECTRA_STORAGE_ENABLE_SD 0
#endif

#define SPECTRA_STORAGE_ENABLE_SD CONFIG_SPECTRA_STORAGE_ENABLE_SD

#ifndef SOC_GPSPI_SUPPORTED
#define SOC_GPSPI_SUPPORTED 0
#endif

#define SPECTRA_STORAGE_SD_BACKEND_AVAILABLE (SOC_GPSPI_SUPPORTED)
#define SPECTRA_STORAGE_SD_ACTIVE (SPECTRA_STORAGE_ENABLE_SD && SPECTRA_STORAGE_SD_BACKEND_AVAILABLE)

typedef struct {
    bool mounted;
    uint64_t total_bytes;
    uint64_t free_bytes;
} sd_card_status_t;

esp_err_t sd_card_init(void);
esp_err_t sd_card_deinit(void);
bool sd_card_is_mounted(void);

esp_err_t sd_card_write_file(const char *relative_path, const void *data, size_t len);
esp_err_t sd_card_read_file(const char *relative_path, void *out_data, size_t max_len, size_t *out_read_len);
esp_err_t sd_card_list_dir(const char *relative_dir, size_t *out_entries);
esp_err_t sd_card_get_free_space(uint64_t *out_free_bytes, uint64_t *out_total_bytes);
esp_err_t sd_card_get_status(sd_card_status_t *out_status);

#endif  // SD_CARD_H
