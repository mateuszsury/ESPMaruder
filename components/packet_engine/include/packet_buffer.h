#ifndef PACKET_BUFFER_H
#define PACKET_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKET_BUFFER_DEFAULT_CAPACITY_BYTES (32U * 1024U)
#define PACKET_BUFFER_FALLBACK_CAPACITY_BYTES (8U * 1024U)
#define PACKET_BUFFER_MAX_FRAME_BYTES 2048U

typedef enum {
    PKT_TYPE_WIFI = 1,
    PKT_TYPE_BLE = 2,
    PKT_TYPE_802154 = 3,
} packet_type_t;

typedef struct {
    size_t capacity_bytes;
    size_t used_bytes;
    size_t free_bytes;
    size_t high_watermark_bytes;
    uint64_t pushed_packets;
    uint64_t popped_packets;
    uint64_t dropped_packets;
    bool using_psram;
} packet_buffer_stats_t;

typedef struct {
    uint8_t *storage;
    size_t capacity_bytes;
    size_t head;
    size_t tail;
    size_t used_bytes;
    size_t high_watermark_bytes;
    uint64_t pushed_packets;
    uint64_t popped_packets;
    uint64_t dropped_packets;
    bool using_psram;
    bool initialized;
} packet_buffer_t;

esp_err_t packet_buffer_init(packet_buffer_t *buffer, size_t requested_capacity_bytes);
esp_err_t packet_buffer_deinit(packet_buffer_t *buffer);

esp_err_t packet_buffer_push(packet_buffer_t *buffer,
                             const uint8_t *packet,
                             size_t length,
                             uint64_t timestamp_us,
                             packet_type_t type);

esp_err_t packet_buffer_pop(packet_buffer_t *buffer,
                            uint8_t *out_packet,
                            size_t out_packet_capacity,
                            size_t *out_length,
                            uint64_t *out_timestamp_us,
                            packet_type_t *out_type);

esp_err_t packet_buffer_flush(packet_buffer_t *buffer);
esp_err_t packet_buffer_stats(const packet_buffer_t *buffer, packet_buffer_stats_t *out_stats);

#ifdef __cplusplus
}
#endif

#endif  // PACKET_BUFFER_H
