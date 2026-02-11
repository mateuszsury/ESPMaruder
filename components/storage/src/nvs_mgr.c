#include "nvs_mgr.h"

#include <stdbool.h>
#include <string.h>

#include "esp_check.h"
#include "esp_log.h"
#include "nvs.h"

static const char *TAG = "nvs_mgr";
static const char *SPECTRA_NVS_NAMESPACE = "spectra";
static const char *SPECTRA_NVS_SCHEMA_KEY = "schema_ver";
static const int32_t SPECTRA_NVS_SCHEMA_VERSION = 1;

static nvs_handle_t s_handle = 0;
static bool s_initialized = false;

static esp_err_t nvs_mgr_commit(void) {
    esp_err_t err = nvs_commit(s_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t nvs_mgr_init(void) {
    if (s_initialized) {
        return ESP_OK;
    }

    esp_err_t err = nvs_open(SPECTRA_NVS_NAMESPACE, NVS_READWRITE, &s_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return err;
    }

    int32_t schema_ver = 0;
    err = nvs_get_i32(s_handle, SPECTRA_NVS_SCHEMA_KEY, &schema_ver);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Initializing NVS schema version: %ld", (long)SPECTRA_NVS_SCHEMA_VERSION);
        ESP_RETURN_ON_ERROR(nvs_set_i32(s_handle, SPECTRA_NVS_SCHEMA_KEY, SPECTRA_NVS_SCHEMA_VERSION), TAG,
                            "Failed to set schema version");
        ESP_RETURN_ON_ERROR(nvs_mgr_commit(), TAG, "Failed to commit schema version");
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_i32(schema_ver) failed: %s", esp_err_to_name(err));
        nvs_close(s_handle);
        s_handle = 0;
        return err;
    } else {
        ESP_LOGI(TAG, "Loaded NVS schema version: %ld", (long)schema_ver);
    }

    s_initialized = true;
    return ESP_OK;
}

esp_err_t nvs_mgr_deinit(void) {
    if (!s_initialized) {
        return ESP_OK;
    }
    nvs_close(s_handle);
    s_handle = 0;
    s_initialized = false;
    return ESP_OK;
}

esp_err_t nvs_mgr_store_string(const char *key, const char *value) {
    if (!s_initialized || key == NULL || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    ESP_RETURN_ON_ERROR(nvs_set_str(s_handle, key, value), TAG, "nvs_set_str(%s) failed", key);
    return nvs_mgr_commit();
}

esp_err_t nvs_mgr_load_string(const char *key, char *out_value, size_t *inout_len) {
    if (!s_initialized || key == NULL || out_value == NULL || inout_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return nvs_get_str(s_handle, key, out_value, inout_len);
}

esp_err_t nvs_mgr_store_blob(const char *key, const void *data, size_t len) {
    if (!s_initialized || key == NULL || data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    ESP_RETURN_ON_ERROR(nvs_set_blob(s_handle, key, data, len), TAG, "nvs_set_blob(%s) failed", key);
    return nvs_mgr_commit();
}

esp_err_t nvs_mgr_load_blob(const char *key, void *out_data, size_t *inout_len) {
    if (!s_initialized || key == NULL || out_data == NULL || inout_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return nvs_get_blob(s_handle, key, out_data, inout_len);
}

esp_err_t nvs_mgr_store_i32(const char *key, int32_t value) {
    if (!s_initialized || key == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    ESP_RETURN_ON_ERROR(nvs_set_i32(s_handle, key, value), TAG, "nvs_set_i32(%s) failed", key);
    return nvs_mgr_commit();
}

esp_err_t nvs_mgr_load_i32(const char *key, int32_t *out_value) {
    if (!s_initialized || key == NULL || out_value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return nvs_get_i32(s_handle, key, out_value);
}
