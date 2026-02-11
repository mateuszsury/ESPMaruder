#ifndef SPIFFS_MGR_H
#define SPIFFS_MGR_H

#include <stddef.h>
#include <stdbool.h>

#include "esp_err.h"

#define SPECTRA_SPIFFS_BASE_PATH "/spiffs"
#define SPECTRA_SPIFFS_PART_LABEL "spiffs"

esp_err_t spiffs_mgr_init(void);
esp_err_t spiffs_mgr_deinit(void);
bool spiffs_mgr_is_mounted(void);

esp_err_t spiffs_mgr_write_file(const char *relative_path, const void *data, size_t len);
esp_err_t spiffs_mgr_read_file(const char *relative_path, void *out_data, size_t max_len, size_t *out_read_len);
esp_err_t spiffs_mgr_list_dir(const char *relative_dir, size_t *out_entries);

#endif  // SPIFFS_MGR_H

