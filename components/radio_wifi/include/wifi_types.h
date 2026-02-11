#ifndef WIFI_TYPES_H
#define WIFI_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_wifi_types_generic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WIFI_SCAN_BAND_AUTO = 0,
    WIFI_SCAN_BAND_2G,
    WIFI_SCAN_BAND_5G,
} wifi_scan_band_t;

typedef struct {
    wifi_scan_band_t band;
    size_t max_results;
    bool show_hidden;
    uint32_t scan_timeout_ms;
} wifi_scan_params_t;

typedef struct {
    char ssid[33];
    uint8_t bssid[6];
    int8_t rssi;
    uint8_t channel;
    wifi_auth_mode_t authmode;
    wifi_cipher_type_t pairwise_cipher;
    wifi_cipher_type_t group_cipher;
    wifi_scan_band_t band;
} wifi_ap_info_t;

#ifdef __cplusplus
}
#endif

#endif  // WIFI_TYPES_H
