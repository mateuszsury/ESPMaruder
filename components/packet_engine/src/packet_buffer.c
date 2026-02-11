#include "packet_buffer.h"

#include <string.h>

#include "esp_heap_caps.h"
#include "esp_log.h"

typedef struct {
    uint16_t length_bytes;
    uint8_t type;
    uint8_t reserved;
    uint64_t timestamp_us;
} packet_record_header_t;

static const char *TAG = "packet_buffer";

static bool packet_type_is_valid(packet_type_t type) {
    return type == PKT_TYPE_WIFI || type == PKT_TYPE_BLE || type == PKT_TYPE_802154;
}

static size_t packet_record_size(size_t payload_size) {
    return sizeof(packet_record_header_t) + payload_size;
}

static void packet_buffer_compact_tail(packet_buffer_t *buffer) {
    const size_t header_size = sizeof(packet_record_header_t);

    while (buffer->used_bytes > 0) {
        const size_t contiguous = buffer->capacity_bytes - buffer->tail;
        if (contiguous < header_size) {
            buffer->used_bytes -= contiguous;
            buffer->tail = 0;
            continue;
        }

        packet_record_header_t hdr = {0};
        memcpy(&hdr, buffer->storage + buffer->tail, header_size);
        if (hdr.length_bytes == 0) {
            buffer->used_bytes -= contiguous;
            buffer->tail = 0;
            continue;
        }

        const size_t rec_size = packet_record_size(hdr.length_bytes);
        if (!packet_type_is_valid((packet_type_t)hdr.type) ||
            hdr.length_bytes > PACKET_BUFFER_MAX_FRAME_BYTES ||
            rec_size > contiguous ||
            rec_size > buffer->used_bytes) {
            ESP_LOGW(TAG, "Buffer corruption detected, flushing queue");
            buffer->head = 0;
            buffer->tail = 0;
            buffer->used_bytes = 0;
            return;
        }

        break;
    }
}

esp_err_t packet_buffer_init(packet_buffer_t *buffer, size_t requested_capacity_bytes) {
    if (buffer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (buffer->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    size_t capacity = requested_capacity_bytes;
    if (capacity == 0) {
        capacity = PACKET_BUFFER_DEFAULT_CAPACITY_BYTES;
    }
    if (capacity < packet_record_size(1)) {
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t *storage = heap_caps_malloc(capacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    bool using_psram = true;

    if (storage == NULL) {
        size_t fallback_capacity = capacity;
        if (fallback_capacity > PACKET_BUFFER_FALLBACK_CAPACITY_BYTES) {
            fallback_capacity = PACKET_BUFFER_FALLBACK_CAPACITY_BYTES;
        }

        storage = heap_caps_malloc(fallback_capacity, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (storage == NULL) {
            return ESP_ERR_NO_MEM;
        }

        capacity = fallback_capacity;
        using_psram = false;
        ESP_LOGW(TAG, "PSRAM unavailable, using internal memory buffer: %zu bytes", capacity);
    }

    memset(storage, 0, capacity);
    memset(buffer, 0, sizeof(*buffer));
    buffer->storage = storage;
    buffer->capacity_bytes = capacity;
    buffer->using_psram = using_psram;
    buffer->initialized = true;

    ESP_LOGI(TAG, "Initialized: capacity=%zu using_psram=%s", capacity, using_psram ? "yes" : "no");
    return ESP_OK;
}

esp_err_t packet_buffer_deinit(packet_buffer_t *buffer) {
    if (buffer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!buffer->initialized) {
        return ESP_OK;
    }

    heap_caps_free(buffer->storage);
    memset(buffer, 0, sizeof(*buffer));
    return ESP_OK;
}

esp_err_t packet_buffer_push(packet_buffer_t *buffer,
                             const uint8_t *packet,
                             size_t length,
                             uint64_t timestamp_us,
                             packet_type_t type) {
    if (buffer == NULL || packet == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!buffer->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!packet_type_is_valid(type) || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (length > PACKET_BUFFER_MAX_FRAME_BYTES || length > UINT16_MAX) {
        return ESP_ERR_INVALID_SIZE;
    }

    const size_t rec_size = packet_record_size(length);
    if (rec_size > buffer->capacity_bytes) {
        buffer->dropped_packets++;
        return ESP_ERR_NO_MEM;
    }

    size_t free_bytes = buffer->capacity_bytes - buffer->used_bytes;
    if (free_bytes < rec_size) {
        buffer->dropped_packets++;
        return ESP_ERR_NO_MEM;
    }

    size_t head = buffer->head;
    size_t contiguous = buffer->capacity_bytes - head;
    if (rec_size > contiguous) {
        if (free_bytes < rec_size + contiguous) {
            buffer->dropped_packets++;
            return ESP_ERR_NO_MEM;
        }

        if (contiguous > 0) {
            memset(buffer->storage + head, 0, contiguous);
            buffer->used_bytes += contiguous;
        }

        head = 0;
    }

    packet_record_header_t hdr = {
        .length_bytes = (uint16_t)length,
        .type = (uint8_t)type,
        .reserved = 0,
        .timestamp_us = timestamp_us,
    };

    memcpy(buffer->storage + head, &hdr, sizeof(hdr));
    memcpy(buffer->storage + head + sizeof(hdr), packet, length);

    head += rec_size;
    if (head >= buffer->capacity_bytes) {
        head = 0;
    }
    buffer->head = head;
    buffer->used_bytes += rec_size;
    buffer->pushed_packets++;

    if (buffer->used_bytes > buffer->high_watermark_bytes) {
        buffer->high_watermark_bytes = buffer->used_bytes;
    }

    return ESP_OK;
}

esp_err_t packet_buffer_pop(packet_buffer_t *buffer,
                            uint8_t *out_packet,
                            size_t out_packet_capacity,
                            size_t *out_length,
                            uint64_t *out_timestamp_us,
                            packet_type_t *out_type) {
    if (buffer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!buffer->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    packet_buffer_compact_tail(buffer);
    if (buffer->used_bytes == 0) {
        return ESP_ERR_NOT_FOUND;
    }

    const size_t header_size = sizeof(packet_record_header_t);
    packet_record_header_t hdr = {0};
    memcpy(&hdr, buffer->storage + buffer->tail, header_size);

    const size_t rec_size = packet_record_size(hdr.length_bytes);
    if (rec_size > buffer->used_bytes || rec_size > (buffer->capacity_bytes - buffer->tail)) {
        ESP_LOGW(TAG, "Inconsistent record at tail, flushing queue");
        buffer->head = 0;
        buffer->tail = 0;
        buffer->used_bytes = 0;
        return ESP_ERR_INVALID_STATE;
    }

    if (out_length != NULL) {
        *out_length = hdr.length_bytes;
    }
    if (out_timestamp_us != NULL) {
        *out_timestamp_us = hdr.timestamp_us;
    }
    if (out_type != NULL) {
        *out_type = (packet_type_t)hdr.type;
    }

    if (out_packet == NULL || out_packet_capacity < hdr.length_bytes) {
        return ESP_ERR_INVALID_SIZE;
    }

    memcpy(out_packet, buffer->storage + buffer->tail + header_size, hdr.length_bytes);

    buffer->tail += rec_size;
    if (buffer->tail >= buffer->capacity_bytes) {
        buffer->tail = 0;
    }
    buffer->used_bytes -= rec_size;
    buffer->popped_packets++;

    return ESP_OK;
}

esp_err_t packet_buffer_flush(packet_buffer_t *buffer) {
    if (buffer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!buffer->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    buffer->head = 0;
    buffer->tail = 0;
    buffer->used_bytes = 0;
    return ESP_OK;
}

esp_err_t packet_buffer_stats(const packet_buffer_t *buffer, packet_buffer_stats_t *out_stats) {
    if (buffer == NULL || out_stats == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!buffer->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    out_stats->capacity_bytes = buffer->capacity_bytes;
    out_stats->used_bytes = buffer->used_bytes;
    out_stats->free_bytes = buffer->capacity_bytes - buffer->used_bytes;
    out_stats->high_watermark_bytes = buffer->high_watermark_bytes;
    out_stats->pushed_packets = buffer->pushed_packets;
    out_stats->popped_packets = buffer->popped_packets;
    out_stats->dropped_packets = buffer->dropped_packets;
    out_stats->using_psram = buffer->using_psram;

    return ESP_OK;
}
