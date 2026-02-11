#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t touch_init(void);
esp_err_t touch_read_raw(uint16_t *x, uint16_t *y, bool *pressed);
esp_err_t touch_calibrate(void);
esp_err_t touch_read_calibrated(uint16_t *x, uint16_t *y, bool *pressed);
bool touch_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif  // TOUCH_DRIVER_H
