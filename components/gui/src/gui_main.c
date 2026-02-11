#include "gui_main.h"

#include "esp_check.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "gui_events.h"
#include "gui_theme.h"
#include "sdkconfig.h"
#include "tft_driver.h"
#include "touch_driver.h"

static const char *TAG = "gui_main";
#if CONFIG_SPECTRA_GUI_ENABLE
static const UBaseType_t GUI_TASK_PRIORITY = 3;
static const uint32_t GUI_TASK_STACK_SIZE = 8192;
static const TickType_t GUI_LOOP_TIMEOUT = pdMS_TO_TICKS(20);

static TaskHandle_t s_gui_task = NULL;
static SemaphoreHandle_t s_gui_mutex = NULL;
static volatile bool s_gui_initialized = false;
static volatile bool s_gui_running = false;
static volatile uint16_t s_active_screen = 0;

static const char *toast_type_to_str(gui_toast_type_t type) {
    switch (type) {
        case GUI_TOAST_INFO:
            return "INFO";
        case GUI_TOAST_WARNING:
            return "WARN";
        case GUI_TOAST_ERROR:
            return "ERR";
        default:
            return "UNK";
    }
}

static void gui_task(void *arg) {
    (void)arg;

    while (s_gui_running) {
        gui_event_t evt = {0};
        if (gui_event_receive(&evt, GUI_LOOP_TIMEOUT) == ESP_OK) {
            ESP_LOGD(TAG, "GUI evt=%d len=%u q_depth=%u",
                     (int)evt.type, (unsigned)evt.data_len, (unsigned)gui_event_queue_depth());
        }
    }

    s_gui_task = NULL;
    vTaskDelete(NULL);
}
#endif

esp_err_t gui_init(void) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    ESP_LOGI(TAG, "GUI disabled by config");
    return ESP_OK;
#else
    if (s_gui_initialized) {
        return ESP_OK;
    }

    s_gui_mutex = xSemaphoreCreateMutex();
    if (s_gui_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = gui_events_init(GUI_EVENT_DEFAULT_QUEUE_LEN);
    if (err != ESP_OK) {
        vSemaphoreDelete(s_gui_mutex);
        s_gui_mutex = NULL;
        return err;
    }

    gui_theme_apply(GUI_THEME_DARK);

    err = tft_init();
    if (err != ESP_OK && err != ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "tft_init failed: %s", esp_err_to_name(err));
    }

    err = touch_init();
    if (err != ESP_OK && err != ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "touch_init failed: %s", esp_err_to_name(err));
    }

    s_gui_initialized = true;
    s_active_screen = 0;
    ESP_LOGI(TAG, "GUI foundation initialized");
    return gui_start();
#endif
}

esp_err_t gui_deinit(void) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    return ESP_OK;
#else
    if (!s_gui_initialized) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(gui_stop(), TAG, "gui_stop failed");
    gui_events_deinit();

    if (s_gui_mutex != NULL) {
        vSemaphoreDelete(s_gui_mutex);
        s_gui_mutex = NULL;
    }

    s_gui_initialized = false;
    return ESP_OK;
#endif
}

esp_err_t gui_start(void) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    return ESP_OK;
#else
    if (!s_gui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_gui_running) {
        return ESP_OK;
    }

    s_gui_running = true;
    if (xTaskCreate(gui_task, "spectra_gui", GUI_TASK_STACK_SIZE, NULL, GUI_TASK_PRIORITY, &s_gui_task) != pdPASS) {
        s_gui_running = false;
        s_gui_task = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "GUI task started");
    return ESP_OK;
#endif
}

esp_err_t gui_stop(void) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    return ESP_OK;
#else
    if (!s_gui_running) {
        return ESP_OK;
    }

    s_gui_running = false;

    for (int i = 0; i < 20; i++) {
        if (s_gui_task == NULL) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (s_gui_task != NULL) {
        vTaskDelete(s_gui_task);
        s_gui_task = NULL;
    }

    ESP_LOGI(TAG, "GUI task stopped");
    return ESP_OK;
#endif
}

bool gui_is_running(void) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    return false;
#else
    return s_gui_running;
#endif
}

esp_err_t gui_navigate_to(uint16_t screen_id) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    (void)screen_id;
    return ESP_OK;
#else
    if (!s_gui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    s_active_screen = screen_id;
    ESP_LOGI(TAG, "Navigate to screen %u", (unsigned)screen_id);
    return ESP_OK;
#endif
}

esp_err_t gui_show_toast(const char *msg, gui_toast_type_t type) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    (void)msg;
    (void)type;
    return ESP_OK;
#else
    if (!s_gui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (msg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "[%s] %s", toast_type_to_str(type), msg);
    return ESP_OK;
#endif
}

esp_err_t gui_lock(TickType_t timeout_ticks) {
#if !CONFIG_SPECTRA_GUI_ENABLE
    (void)timeout_ticks;
    return ESP_OK;
#else
    if (s_gui_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return (xSemaphoreTake(s_gui_mutex, timeout_ticks) == pdTRUE) ? ESP_OK : ESP_ERR_TIMEOUT;
#endif
}

void gui_unlock(void) {
#if CONFIG_SPECTRA_GUI_ENABLE
    if (s_gui_mutex != NULL) {
        xSemaphoreGive(s_gui_mutex);
    }
#endif
}
