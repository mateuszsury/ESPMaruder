#include "touch_driver.h"

#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "touch_driver";
static bool s_ready = false;

esp_err_t touch_init(void) {
#if CONFIG_SPECTRA_GUI_ENABLE_HW_DRIVERS
    s_ready = false;
    ESP_LOGW(TAG, "Touch hardware driver initialization is not implemented yet");
    return ESP_ERR_NOT_SUPPORTED;
#else
    s_ready = false;
    ESP_LOGI(TAG, "Touch driver skipped (hardware drivers disabled)");
    return ESP_OK;
#endif
}

esp_err_t touch_read_raw(uint16_t *x, uint16_t *y, bool *pressed) {
    if (x == NULL || y == NULL || pressed == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_ready) {
        *x = 0;
        *y = 0;
        *pressed = false;
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t touch_calibrate(void) {
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t touch_read_calibrated(uint16_t *x, uint16_t *y, bool *pressed) {
    return touch_read_raw(x, y, pressed);
}

bool touch_is_ready(void) {
    return s_ready;
}
