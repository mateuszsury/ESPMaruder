#include "tft_driver.h"

#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "tft_driver";
static bool s_ready = false;
static bool s_sleep = false;

esp_err_t tft_init(void) {
#if CONFIG_SPECTRA_GUI_ENABLE_HW_DRIVERS
    s_ready = false;
    s_sleep = false;
    ESP_LOGW(TAG, "TFT hardware driver initialization is not implemented yet");
    return ESP_ERR_NOT_SUPPORTED;
#else
    s_ready = false;
    s_sleep = false;
    ESP_LOGI(TAG, "TFT driver skipped (hardware drivers disabled)");
    return ESP_OK;
#endif
}

esp_err_t tft_set_backlight(uint8_t brightness) {
    (void)brightness;
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t tft_flush_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const void *color_data, size_t data_len) {
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)color_data;
    (void)data_len;
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t tft_sleep(void) {
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    s_sleep = true;
    return ESP_OK;
}

esp_err_t tft_wake(void) {
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    s_sleep = false;
    return ESP_OK;
}

bool tft_is_ready(void) {
    return s_ready && !s_sleep;
}
