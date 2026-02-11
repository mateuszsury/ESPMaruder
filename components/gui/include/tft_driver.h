#ifndef TFT_DRIVER_H
#define TFT_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t tft_init(void);
esp_err_t tft_set_backlight(uint8_t brightness);
esp_err_t tft_flush_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const void *color_data, size_t data_len);
esp_err_t tft_sleep(void);
esp_err_t tft_wake(void);
bool tft_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif  // TFT_DRIVER_H
