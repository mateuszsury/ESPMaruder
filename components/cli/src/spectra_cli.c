#include "spectra_cli.h"

#include "cli_engine.h"
#include "esp_log.h"

static const char *TAG = "spectra_cli";

esp_err_t spectra_cli_init(void) {
    esp_err_t err = cli_engine_init();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CLI bootstrap initialized");
    }
    return err;
}
