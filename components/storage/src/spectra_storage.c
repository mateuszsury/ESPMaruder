#include "spectra_storage.h"

#include <string.h>

#include "esp_check.h"
#include "esp_log.h"

#include "log_rotate.h"
#include "nvs_mgr.h"
#include "sd_card.h"
#include "spiffs_mgr.h"

static const char *TAG = "spectra_storage";

static spectra_storage_health_t s_health = {
    .nvs_ready = false,
    .spiffs_ready = false,
    .sd_ready = false,
    .sd_total_bytes = 0,
    .sd_free_bytes = 0,
};

esp_err_t spectra_storage_init(void) {
    memset(&s_health, 0, sizeof(s_health));

    ESP_RETURN_ON_ERROR(nvs_mgr_init(), TAG, "NVS manager init failed");
    s_health.nvs_ready = true;

    esp_err_t err = spiffs_mgr_init();
    if (err == ESP_OK) {
        s_health.spiffs_ready = true;
    } else {
        ESP_LOGW(TAG, "SPIFFS init failed: err=%d", err);
    }

    err = sd_card_init();
    if (err == ESP_OK) {
        s_health.sd_ready = true;
        (void)sd_card_get_free_space(&s_health.sd_free_bytes, &s_health.sd_total_bytes);

        if (log_rotate_check(s_health.sd_free_bytes) == ESP_ERR_NO_MEM) {
            size_t deleted = 0;
            (void)log_rotate_cleanup("captures", 16, &deleted);
            (void)sd_card_get_free_space(&s_health.sd_free_bytes, &s_health.sd_total_bytes);
            ESP_LOGW(TAG, "SD low-space cleanup executed, deleted=%u", (unsigned)deleted);
        }
    } else {
        if (!SPECTRA_STORAGE_ENABLE_SD) {
            ESP_LOGI(TAG, "SD disabled by config");
        } else if (!SPECTRA_STORAGE_SD_BACKEND_AVAILABLE) {
            ESP_LOGW(TAG, "SD enabled in config but unavailable on current target/profile (GPSPI unsupported)");
        } else {
            ESP_LOGW(TAG, "SD card not mounted: err=%d", err);
        }
    }

    ESP_LOGI(TAG,
             "Storage initialized: nvs=%s spiffs=%s sd=%s free=%llu total=%llu",
             s_health.nvs_ready ? "yes" : "no",
             s_health.spiffs_ready ? "yes" : "no",
             s_health.sd_ready ? "yes" : "no",
             (unsigned long long)s_health.sd_free_bytes,
             (unsigned long long)s_health.sd_total_bytes);
    return ESP_OK;
}

esp_err_t spectra_storage_deinit(void) {
    (void)sd_card_deinit();
    (void)spiffs_mgr_deinit();
    (void)nvs_mgr_deinit();
    memset(&s_health, 0, sizeof(s_health));
    return ESP_OK;
}

esp_err_t spectra_storage_get_health(spectra_storage_health_t *out_health) {
    if (out_health == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_health = s_health;

    if (s_health.sd_ready) {
        (void)sd_card_get_free_space(&out_health->sd_free_bytes, &out_health->sd_total_bytes);
    }
    return ESP_OK;
}
