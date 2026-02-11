#ifndef WIFI_ENGINE_H
#define WIFI_ENGINE_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_wifi_types_generic.h"
#include "wifi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_engine_init(void);
esp_err_t wifi_engine_start(void);
esp_err_t wifi_engine_stop(void);
esp_err_t wifi_engine_deinit(void);
bool wifi_engine_is_initialized(void);

esp_err_t wifi_engine_set_band(wifi_scan_band_t band);
esp_err_t wifi_engine_set_channel(uint8_t channel, wifi_second_chan_t secondary);
esp_err_t wifi_engine_get_mac(uint8_t mac_out[6]);
esp_err_t wifi_engine_set_mac(const uint8_t mac[6]);

#ifdef __cplusplus
}
#endif

#endif  // WIFI_ENGINE_H
