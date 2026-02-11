#include "spiffs_mgr.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "esp_check.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "spiffs_mgr";

static bool s_spiffs_mounted = false;

static esp_err_t make_spiffs_path(const char *relative_path, char *out_path, size_t out_path_len) {
    if (relative_path == NULL || out_path == NULL || out_path_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    int written = 0;
    if (relative_path[0] == '/') {
        written = snprintf(out_path, out_path_len, "%s%s", SPECTRA_SPIFFS_BASE_PATH, relative_path);
    } else {
        written = snprintf(out_path, out_path_len, "%s/%s", SPECTRA_SPIFFS_BASE_PATH, relative_path);
    }
    if (written < 0 || (size_t)written >= out_path_len) {
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

esp_err_t spiffs_mgr_init(void) {
    if (s_spiffs_mounted) {
        return ESP_OK;
    }

    esp_vfs_spiffs_conf_t conf = {
        .base_path = SPECTRA_SPIFFS_BASE_PATH,
        .partition_label = SPECTRA_SPIFFS_PART_LABEL,
        .max_files = 8,
        .format_if_mount_failed = false,
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed: %s", esp_err_to_name(err));
        return err;
    }

    size_t total = 0;
    size_t used = 0;
    err = esp_spiffs_info(SPECTRA_SPIFFS_PART_LABEL, &total, &used);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS mounted: total=%u, used=%u", (unsigned)total, (unsigned)used);
    } else {
        ESP_LOGW(TAG, "SPIFFS info unavailable: %s", esp_err_to_name(err));
    }

    s_spiffs_mounted = true;
    return ESP_OK;
}

esp_err_t spiffs_mgr_deinit(void) {
    if (!s_spiffs_mounted) {
        return ESP_OK;
    }
    esp_vfs_spiffs_unregister(SPECTRA_SPIFFS_PART_LABEL);
    s_spiffs_mounted = false;
    return ESP_OK;
}

bool spiffs_mgr_is_mounted(void) { return s_spiffs_mounted; }

esp_err_t spiffs_mgr_write_file(const char *relative_path, const void *data, size_t len) {
    if (!s_spiffs_mounted || relative_path == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char path[192];
    ESP_RETURN_ON_ERROR(make_spiffs_path(relative_path, path, sizeof(path)), TAG, "Invalid SPIFFS path");

    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        ESP_LOGE(TAG, "Failed to open %s for write", path);
        return ESP_FAIL;
    }

    size_t written = fwrite(data, 1, len, fp);
    fclose(fp);

    if (written != len) {
        ESP_LOGE(TAG, "Incomplete write to %s: expected=%u, got=%u", path, (unsigned)len, (unsigned)written);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t spiffs_mgr_read_file(const char *relative_path, void *out_data, size_t max_len, size_t *out_read_len) {
    if (!s_spiffs_mounted || relative_path == NULL || out_data == NULL || out_read_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char path[192];
    ESP_RETURN_ON_ERROR(make_spiffs_path(relative_path, path, sizeof(path)), TAG, "Invalid SPIFFS path");

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    size_t read_len = fread(out_data, 1, max_len, fp);
    fclose(fp);
    *out_read_len = read_len;
    return ESP_OK;
}

esp_err_t spiffs_mgr_list_dir(const char *relative_dir, size_t *out_entries) {
    if (!s_spiffs_mounted || relative_dir == NULL || out_entries == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char path[192];
    ESP_RETURN_ON_ERROR(make_spiffs_path(relative_dir, path, sizeof(path)), TAG, "Invalid SPIFFS directory");

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
