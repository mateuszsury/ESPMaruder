#include "wifi_engine.h"

#include <string.h>

#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

static const char *TAG = "wifi_engine";

static bool s_initialized = false;
static bool s_started = false;
static esp_netif_t *s_sta_netif = NULL;
static esp_netif_t *s_ap_netif = NULL;

static esp_err_t init_netif_stack(void) {
    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    return ESP_OK;
}

esp_err_t wifi_engine_init(void) {
    if (s_initialized) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(init_netif_stack(), TAG, "netif stack init failed");

    s_sta_netif = esp_netif_create_default_wifi_sta();
    s_ap_netif = esp_netif_create_default_wifi_ap();
    ESP_RETURN_ON_FALSE(s_sta_netif != NULL && s_ap_netif != NULL, ESP_FAIL, TAG, "wifi netif create failed");

    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        return err;
    }

    ESP_RETURN_ON_ERROR(esp_wifi_set_storage(WIFI_STORAGE_RAM), TAG, "set storage failed");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "set mode failed");

    s_initialized = true;
    s_started = false;
    ESP_LOGI(TAG, "Wi-Fi engine initialized");
    return ESP_OK;
}

esp_err_t wifi_engine_start(void) {
    ESP_RETURN_ON_FALSE(s_initialized, ESP_ERR_INVALID_STATE, TAG, "engine not initialized");
    if (s_started) {
        return ESP_OK;
    }

    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_WIFI_CONN) {
        return err;
    }

    s_started = true;
    return ESP_OK;
}

esp_err_t wifi_engine_stop(void) {
    if (!s_initialized || !s_started) {
        return ESP_OK;
    }

    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_INIT && err != ESP_ERR_WIFI_NOT_STARTED) {
        return err;
    }

    s_started = false;
    return ESP_OK;
}

esp_err_t wifi_engine_deinit(void) {
    if (!s_initialized) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(wifi_engine_stop(), TAG, "wifi stop failed");

    esp_err_t err = esp_wifi_deinit();
    if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_INIT) {
        return err;
    }

    if (s_sta_netif != NULL) {
        esp_netif_destroy_default_wifi(s_sta_netif);
        s_sta_netif = NULL;
    }
    if (s_ap_netif != NULL) {
        esp_netif_destroy_default_wifi(s_ap_netif);
        s_ap_netif = NULL;
    }

    s_initialized = false;
    s_started = false;
    return ESP_OK;
}

bool wifi_engine_is_initialized(void) {
    return s_initialized;
}

esp_err_t wifi_engine_set_band(wifi_scan_band_t band) {
    ESP_RETURN_ON_FALSE(s_initialized, ESP_ERR_INVALID_STATE, TAG, "engine not initialized");

#if defined(WIFI_BAND_MODE_2G_ONLY) && defined(WIFI_BAND_MODE_5G_ONLY) && defined(WIFI_BAND_MODE_AUTO)
    wifi_band_mode_t mode = WIFI_BAND_MODE_AUTO;
    switch (band) {
        case WIFI_SCAN_BAND_2G:
            mode = WIFI_BAND_MODE_2G_ONLY;
            break;
        case WIFI_SCAN_BAND_5G:
            mode = WIFI_BAND_MODE_5G_ONLY;
            break;
        case WIFI_SCAN_BAND_AUTO:
        default:
            mode = WIFI_BAND_MODE_AUTO;
            break;
    }
    return esp_wifi_set_band_mode(mode);
#else
    if (band == WIFI_SCAN_BAND_5G) {
        return ESP_ERR_NOT_SUPPORTED;
    }
    return ESP_OK;
#endif
}

esp_err_t wifi_engine_set_channel(uint8_t channel, wifi_second_chan_t secondary) {
    ESP_RETURN_ON_FALSE(s_initialized, ESP_ERR_INVALID_STATE, TAG, "engine not initialized");
    return esp_wifi_set_channel(channel, secondary);
}

esp_err_t wifi_engine_get_mac(uint8_t mac_out[6]) {
    ESP_RETURN_ON_FALSE(s_initialized, ESP_ERR_INVALID_STATE, TAG, "engine not initialized");
    ESP_RETURN_ON_FALSE(mac_out != NULL, ESP_ERR_INVALID_ARG, TAG, "mac_out is null");
    return esp_wifi_get_mac(WIFI_IF_STA, mac_out);
}

esp_err_t wifi_engine_set_mac(const uint8_t mac[6]) {
    ESP_RETURN_ON_FALSE(s_initialized, ESP_ERR_INVALID_STATE, TAG, "engine not initialized");
    ESP_RETURN_ON_FALSE(mac != NULL, ESP_ERR_INVALID_ARG, TAG, "mac is null");

    uint8_t mac_copy[6];
    memcpy(mac_copy, mac, sizeof(mac_copy));
    return esp_wifi_set_mac(WIFI_IF_STA, mac_copy);
}
