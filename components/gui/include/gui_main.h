#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GUI_TOAST_INFO = 0,
    GUI_TOAST_WARNING,
    GUI_TOAST_ERROR,
} gui_toast_type_t;

esp_err_t gui_init(void);
esp_err_t gui_deinit(void);
esp_err_t gui_start(void);
esp_err_t gui_stop(void);
bool gui_is_running(void);

esp_err_t gui_navigate_to(uint16_t screen_id);
esp_err_t gui_show_toast(const char *msg, gui_toast_type_t type);

esp_err_t gui_lock(TickType_t timeout_ticks);
void gui_unlock(void);

#ifdef __cplusplus
}
#endif

#endif  // GUI_MAIN_H
