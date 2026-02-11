#include "led_status.h"

#include <stdbool.h>

#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static const char *TAG = "led_status";
static const TickType_t LED_TIMER_PERIOD = pdMS_TO_TICKS(100);

static TimerHandle_t s_led_timer = NULL;
static gpio_num_t s_led_pin = GPIO_NUM_NC;
static volatile led_mode_t s_mode = LED_MODE_IDLE;
static volatile bool s_initialized = false;
static uint32_t s_tick = 0;

static void led_apply_level(bool on) {
    if (!s_initialized || s_led_pin == GPIO_NUM_NC) {
        return;
    }
    gpio_set_level(s_led_pin, on ? 1 : 0);
}

static bool led_level_for_mode(led_mode_t mode, uint32_t tick) {
    switch (mode) {
        case LED_MODE_IDLE:
            return (tick % 12U) == 0U;
        case LED_MODE_SCANNING:
            return (tick % 2U) == 0U;
        case LED_MODE_ATTACKING:
            return true;
        case LED_MODE_CAPTURING: {
            const uint32_t phase = tick % 10U;
            return phase == 0U || phase == 2U;
        }
        default:
            return false;
    }
}

static void led_timer_cb(TimerHandle_t timer) {
    (void)timer;
    if (!s_initialized) {
        return;
    }

    s_tick++;
    const bool on = led_level_for_mode(s_mode, s_tick);
    led_apply_level(on);
}

esp_err_t led_status_init(gpio_num_t pin) {
    if (pin == GPIO_NUM_NC) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_initialized) {
        return ESP_OK;
    }

    const gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "gpio_config failed");

    s_led_pin = pin;
    s_tick = 0;

    s_led_timer = xTimerCreate("spectra_led", LED_TIMER_PERIOD, pdTRUE, NULL, led_timer_cb);
    if (s_led_timer == NULL) {
        s_led_pin = GPIO_NUM_NC;
        return ESP_ERR_NO_MEM;
    }

    if (xTimerStart(s_led_timer, 0) != pdPASS) {
        xTimerDelete(s_led_timer, 0);
        s_led_timer = NULL;
        s_led_pin = GPIO_NUM_NC;
        return ESP_FAIL;
    }

    s_mode = LED_MODE_IDLE;
    s_initialized = true;
    led_apply_level(false);
    ESP_LOGI(TAG, "LED status initialized on GPIO %d", (int)pin);
    return ESP_OK;
}

esp_err_t led_status_deinit(void) {
    if (!s_initialized) {
        return ESP_OK;
    }

    if (s_led_timer != NULL) {
        xTimerStop(s_led_timer, 0);
        xTimerDelete(s_led_timer, 0);
        s_led_timer = NULL;
    }

    led_apply_level(false);
    gpio_reset_pin(s_led_pin);
    s_led_pin = GPIO_NUM_NC;
    s_initialized = false;
    s_mode = LED_MODE_IDLE;
    s_tick = 0;
    return ESP_OK;
}

esp_err_t led_set_mode(led_mode_t mode) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (mode < LED_MODE_IDLE || mode > LED_MODE_CAPTURING) {
        return ESP_ERR_INVALID_ARG;
    }

    s_mode = mode;
    const bool on = led_level_for_mode(s_mode, s_tick);
    led_apply_level(on);
    return ESP_OK;
}

led_mode_t led_get_mode(void) {
    return s_mode;
}
