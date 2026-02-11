#ifndef LOG_ROTATE_H
#define LOG_ROTATE_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#define LOG_ROTATE_MIN_FREE_MB 50U

esp_err_t log_rotate_check(uint64_t free_bytes);
esp_err_t log_rotate_cleanup(const char *capture_dir, size_t max_delete, size_t *out_deleted);

#endif  // LOG_ROTATE_H

