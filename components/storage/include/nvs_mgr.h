#ifndef NVS_MGR_H
#define NVS_MGR_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

esp_err_t nvs_mgr_init(void);
esp_err_t nvs_mgr_deinit(void);

esp_err_t nvs_mgr_store_string(const char *key, const char *value);
esp_err_t nvs_mgr_load_string(const char *key, char *out_value, size_t *inout_len);

esp_err_t nvs_mgr_store_blob(const char *key, const void *data, size_t len);
esp_err_t nvs_mgr_load_blob(const char *key, void *out_data, size_t *inout_len);

esp_err_t nvs_mgr_store_i32(const char *key, int32_t value);
esp_err_t nvs_mgr_load_i32(const char *key, int32_t *out_value);

#endif  // NVS_MGR_H

