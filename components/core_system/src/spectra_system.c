#include "spectra_system.h"

#include "esp_log.h"

static const char *TAG = "spectra_system";

esp_err_t spectra_system_init(void) {
    ESP_LOGI(TAG, "Core system bootstrap initialized");
    return ESP_OK;
}

