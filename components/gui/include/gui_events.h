#ifndef GUI_EVENTS_H
#define GUI_EVENTS_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GUI_EVENT_MAX_DATA_LEN 96U
#define GUI_EVENT_DEFAULT_QUEUE_LEN 128U

typedef enum {
    GUI_EVT_WIFI_SCAN_RESULT = 0,
    GUI_EVT_WIFI_SCAN_DONE,
    GUI_EVT_WIFI_SNIFFER_PACKET,
    GUI_EVT_WIFI_HANDSHAKE,
    GUI_EVT_WIFI_ROGUE_DETECTED,
    GUI_EVT_BLE_DEVICE_FOUND,
    GUI_EVT_BLE_GATT_RESULT,
    GUI_EVT_ZB_NETWORK_FOUND,
    GUI_EVT_ZB_FRAME_DECODED,
    GUI_EVT_THREAD_NETWORK_FOUND,
    GUI_EVT_GPS_FIX,
    GUI_EVT_ATTACK_STATUS,
    GUI_EVT_SD_SPACE_LOW,
    GUI_EVT_SYSTEM_ERROR,
} gui_event_type_t;

typedef struct {
    gui_event_type_t type;
    uint16_t data_len;
    uint64_t timestamp_us;
    uint8_t data[GUI_EVENT_MAX_DATA_LEN];
} gui_event_t;

esp_err_t gui_events_init(size_t queue_len);
esp_err_t gui_events_deinit(void);
esp_err_t gui_event_send(gui_event_type_t type, const void *data, size_t data_len, TickType_t timeout_ticks);
esp_err_t gui_event_receive(gui_event_t *out_event, TickType_t timeout_ticks);
size_t gui_event_queue_depth(void);

#ifdef __cplusplus
}
#endif

#endif  // GUI_EVENTS_H
