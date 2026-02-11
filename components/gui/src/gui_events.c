#include "gui_events.h"

#include <string.h>

#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/queue.h"

static QueueHandle_t s_event_queue = NULL;

static bool gui_event_type_valid(gui_event_type_t type) {
    return type >= GUI_EVT_WIFI_SCAN_RESULT && type <= GUI_EVT_SYSTEM_ERROR;
}

esp_err_t gui_events_init(size_t queue_len) {
    if (queue_len == 0) {
        queue_len = GUI_EVENT_DEFAULT_QUEUE_LEN;
    }

    if (s_event_queue != NULL) {
        return ESP_OK;
    }

    s_event_queue = xQueueCreate((UBaseType_t)queue_len, sizeof(gui_event_t));
    if (s_event_queue == NULL) {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t gui_events_deinit(void) {
    if (s_event_queue == NULL) {
        return ESP_OK;
    }

    vQueueDelete(s_event_queue);
    s_event_queue = NULL;
    return ESP_OK;
}

esp_err_t gui_event_send(gui_event_type_t type, const void *data, size_t data_len, TickType_t timeout_ticks) {
    if (s_event_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!gui_event_type_valid(type)) {
        return ESP_ERR_INVALID_ARG;
    }
    if (data_len > GUI_EVENT_MAX_DATA_LEN) {
        return ESP_ERR_INVALID_SIZE;
    }
    if (data_len > 0 && data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    gui_event_t evt = {
        .type = type,
        .data_len = (uint16_t)data_len,
        .timestamp_us = (uint64_t)esp_timer_get_time(),
    };
    if (data_len > 0) {
        memcpy(evt.data, data, data_len);
    }

    if (xQueueSend(s_event_queue, &evt, timeout_ticks) != pdPASS) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

esp_err_t gui_event_receive(gui_event_t *out_event, TickType_t timeout_ticks) {
    if (s_event_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_RETURN_ON_FALSE(out_event != NULL, ESP_ERR_INVALID_ARG, "gui_events", "out_event is null");

    if (xQueueReceive(s_event_queue, out_event, timeout_ticks) != pdPASS) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

size_t gui_event_queue_depth(void) {
    if (s_event_queue == NULL) {
        return 0;
    }
    return (size_t)uxQueueMessagesWaiting(s_event_queue);
}
