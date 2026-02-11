#include "log_rotate.h"

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "esp_check.h"
#include "esp_log.h"

#include "sd_card.h"

static const char *TAG = "log_rotate";

typedef struct {
    char path[256];
    time_t mtime;
} log_file_info_t;

static bool has_pcap_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    return dot != NULL && strcmp(dot, ".pcap") == 0;
}

static int compare_oldest_first(const void *a, const void *b) {
    const log_file_info_t *lhs = (const log_file_info_t *)a;
    const log_file_info_t *rhs = (const log_file_info_t *)b;
    if (lhs->mtime < rhs->mtime) {
        return -1;
    }
    if (lhs->mtime > rhs->mtime) {
        return 1;
    }
    return 0;
}

static esp_err_t make_capture_path(const char *capture_dir, char *out_path, size_t out_path_len) {
    if (capture_dir == NULL || out_path == NULL || out_path_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    int written = 0;
    if (strncmp(capture_dir, SPECTRA_SD_BASE_PATH, strlen(SPECTRA_SD_BASE_PATH)) == 0) {
        written = snprintf(out_path, out_path_len, "%s", capture_dir);
    } else if (capture_dir[0] == '/') {
        written = snprintf(out_path, out_path_len, "%s%s", SPECTRA_SD_BASE_PATH, capture_dir);
    } else {
        written = snprintf(out_path, out_path_len, "%s/%s", SPECTRA_SD_BASE_PATH, capture_dir);
    }

    if (written < 0 || (size_t)written >= out_path_len) {
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

esp_err_t log_rotate_check(uint64_t free_bytes) {
    const uint64_t min_bytes = (uint64_t)LOG_ROTATE_MIN_FREE_MB * 1024ULL * 1024ULL;
    return (free_bytes >= min_bytes) ? ESP_OK : ESP_ERR_NO_MEM;
}

esp_err_t log_rotate_cleanup(const char *capture_dir, size_t max_delete, size_t *out_deleted) {
    if (capture_dir == NULL || out_deleted == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_deleted = 0;

    if (!sd_card_is_mounted()) {
        return ESP_ERR_INVALID_STATE;
    }

    char abs_dir[256];
    ESP_RETURN_ON_ERROR(make_capture_path(capture_dir, abs_dir, sizeof(abs_dir)), TAG, "Invalid capture dir");

    DIR *dir = opendir(abs_dir);
    if (dir == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    log_file_info_t files[128];
    size_t file_count = 0;
    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL && file_count < (sizeof(files) / sizeof(files[0]))) {
        if (!has_pcap_extension(entry->d_name)) {
            continue;
        }

        int path_len = snprintf(files[file_count].path, sizeof(files[file_count].path), "%s/%s", abs_dir, entry->d_name);
        if (path_len <= 0 || (size_t)path_len >= sizeof(files[file_count].path)) {
            continue;
        }

        struct stat st = {0};
        if (stat(files[file_count].path, &st) != 0) {
            continue;
        }
        files[file_count].mtime = st.st_mtime;
        file_count++;
    }
    closedir(dir);

    if (file_count == 0 || max_delete == 0) {
        return ESP_OK;
    }

    qsort(files, file_count, sizeof(files[0]), compare_oldest_first);

    uint64_t free_bytes = 0;
    uint64_t total_bytes = 0;
    (void)sd_card_get_free_space(&free_bytes, &total_bytes);
    const uint64_t threshold = (uint64_t)LOG_ROTATE_MIN_FREE_MB * 1024ULL * 1024ULL;

    size_t deleted = 0;
    for (size_t i = 0; i < file_count && deleted < max_delete; ++i) {
        if (free_bytes >= threshold) {
            break;
        }
        if (remove(files[i].path) == 0) {
            deleted++;
            (void)sd_card_get_free_space(&free_bytes, &total_bytes);
            ESP_LOGW(TAG, "Removed old capture: %s", files[i].path);
        }
    }

    *out_deleted = deleted;
    return ESP_OK;
}

