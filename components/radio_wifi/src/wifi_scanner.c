#include "wifi_scanner.h"

#include <stdlib.h>
#include <string.h>

#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "wifi_engine.h"

static const char *TAG = "wifi_scanner";
static const size_t WIFI_SCAN_DEFAULT_MAX_RESULTS = 128;

static int compare_ap_by_rssi_desc(const void *a, const void *b) {
    const wifi_ap_record_t *lhs = (const wifi_ap_record_t *)a;
    const wifi_ap_record_t *rhs = (const wifi_ap_record_t *)b;
    return (int)rhs->rssi - (int)lhs->rssi;
}

static wifi_scan_band_t band_from_channel(uint8_t channel) {
    return (channel > 14U) ? WIFI_SCAN_BAND_5G : WIFI_SCAN_BAND_2G;
}

static void *psram_first_calloc(size_t count, size_t size) {
    void *ptr = heap_caps_calloc(count, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (ptr != NULL) {
        return ptr;
    }
    return calloc(count, size);
}

static wifi_scan_params_t normalized_params(const wifi_scan_params_t *params) {
    wifi_scan_params_t out = {
        .band = WIFI_SCAN_BAND_AUTO,
        .max_results = WIFI_SCAN_DEFAULT_MAX_RESULTS,
        .show_hidden = true,
        .scan_timeout_ms = 0,
    };

    if (params != NULL) {
        out = *params;
        if (out.max_results == 0) {
            out.max_results = WIFI_SCAN_DEFAULT_MAX_RESULTS;
        }
    }

    return out;
}

esp_err_t wifi_scan_ap(const wifi_scan_params_t *params, wifi_ap_info_t **out_results, size_t *out_count) {
    ESP_RETURN_ON_FALSE(out_results != NULL, ESP_ERR_INVALID_ARG, TAG, "out_results is null");
    ESP_RETURN_ON_FALSE(out_count != NULL, ESP_ERR_INVALID_ARG, TAG, "out_count is null");

    *out_results = NULL;
    *out_count = 0;

    wifi_scan_params_t cfg = normalized_params(params);

    ESP_RETURN_ON_ERROR(wifi_engine_init(), TAG, "wifi_engine_init failed");
    ESP_RETURN_ON_ERROR(wifi_engine_set_band(cfg.band), TAG, "set band failed");
    ESP_RETURN_ON_ERROR(wifi_engine_start(), TAG, "wifi_engine_start failed");

    wifi_scan_config_t scan_cfg = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = cfg.show_hidden,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
            .active = {
                .min = 50,
                .max = 120,
            },
        },
        .home_chan_dwell_time = 30,
    };

    esp_err_t err = esp_wifi_scan_start(&scan_cfg, true);
    if (err != ESP_OK) {
        return err;
    }

    uint16_t ap_count = 0;
    ESP_RETURN_ON_ERROR(esp_wifi_scan_get_ap_num(&ap_count), TAG, "scan_get_ap_num failed");
    if (ap_count == 0) {
        return ESP_OK;
    }

    if ((size_t)ap_count > cfg.max_results) {
        ap_count = (uint16_t)cfg.max_results;
    }

    wifi_ap_record_t *records = calloc(ap_count, sizeof(wifi_ap_record_t));
    ESP_RETURN_ON_FALSE(records != NULL, ESP_ERR_NO_MEM, TAG, "records alloc failed");

    uint16_t records_count = ap_count;
    err = esp_wifi_scan_get_ap_records(&records_count, records);
    if (err != ESP_OK) {
        free(records);
        return err;
    }

    qsort(records, records_count, sizeof(wifi_ap_record_t), compare_ap_by_rssi_desc);

    wifi_ap_info_t *results = psram_first_calloc(records_count, sizeof(wifi_ap_info_t));
    if (results == NULL) {
        free(records);
        return ESP_ERR_NO_MEM;
    }

    for (size_t i = 0; i < records_count; i++) {
        const wifi_ap_record_t *rec = &records[i];
        wifi_ap_info_t *out = &results[i];

        memcpy(out->ssid, rec->ssid, sizeof(rec->ssid));
        out->ssid[sizeof(out->ssid) - 1U] = '\0';
        memcpy(out->bssid, rec->bssid, sizeof(out->bssid));
        out->rssi = rec->rssi;
        out->channel = rec->primary;
        out->authmode = rec->authmode;
        out->pairwise_cipher = rec->pairwise_cipher;
        out->group_cipher = rec->group_cipher;
        out->band = band_from_channel(rec->primary);
    }

    free(records);
    *out_results = results;
    *out_count = records_count;
    ESP_LOGI(TAG, "AP scan done: %u result(s)", (unsigned)records_count);
    return ESP_OK;
}

void wifi_scan_ap_free(wifi_ap_info_t *results) {
    free(results);
}
