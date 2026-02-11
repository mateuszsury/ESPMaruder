#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <stddef.h>

#include "esp_err.h"
#include "wifi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_scan_ap(const wifi_scan_params_t *params, wifi_ap_info_t **out_results, size_t *out_count);
void wifi_scan_ap_free(wifi_ap_info_t *results);

#ifdef __cplusplus
}
#endif

#endif  // WIFI_SCANNER_H
