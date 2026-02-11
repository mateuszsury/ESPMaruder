#ifndef SPECTRA_STORAGE_H
#define SPECTRA_STORAGE_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

typedef struct {
    bool nvs_ready;
    bool spiffs_ready;
    bool sd_ready;
    uint64_t sd_total_bytes;
    uint64_t sd_free_bytes;
} spectra_storage_health_t;

esp_err_t spectra_storage_init(void);
esp_err_t spectra_storage_deinit(void);
esp_err_t spectra_storage_get_health(spectra_storage_health_t *out_health);

#endif  // SPECTRA_STORAGE_H
