#ifndef LED_STATUS_H
#define LED_STATUS_H

#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LED_MODE_IDLE = 0,
    LED_MODE_SCANNING,
    LED_MODE_ATTACKING,
    LED_MODE_CAPTURING,
} led_mode_t;

esp_err_t led_status_init(gpio_num_t pin);
esp_err_t led_status_deinit(void);
esp_err_t led_set_mode(led_mode_t mode);
led_mode_t led_get_mode(void);

#ifdef __cplusplus
}
#endif

#endif  // LED_STATUS_H
