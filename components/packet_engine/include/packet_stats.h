#ifndef PACKET_STATS_H
#define PACKET_STATS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "packet_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKET_STATS_MAX_SUBTYPE 16U
#define PACKET_STATS_RSSI_BIN_DBM 5
#define PACKET_STATS_RSSI_MIN_DBM (-120)
#define PACKET_STATS_RSSI_MAX_DBM (0)
#define PACKET_STATS_RSSI_BIN_COUNT ((PACKET_STATS_RSSI_MAX_DBM - PACKET_STATS_RSSI_MIN_DBM) / PACKET_STATS_RSSI_BIN_DBM + 1)

typedef struct {
    uint64_t total_packets;
    uint64_t total_bytes;
    uint64_t packets_per_type[4];
    uint64_t packets_per_subtype[PACKET_STATS_MAX_SUBTYPE];
    uint64_t rssi_histogram[PACKET_STATS_RSSI_BIN_COUNT];
    uint32_t last_window_pps;
} packet_stats_snapshot_t;

typedef struct {
    uint64_t total_packets;
    uint64_t total_bytes;
    uint64_t packets_per_type[4];
    uint64_t packets_per_subtype[PACKET_STATS_MAX_SUBTYPE];
    uint64_t rssi_histogram[PACKET_STATS_RSSI_BIN_COUNT];
    uint64_t window_start_us;
    uint32_t window_packet_count;
    uint32_t last_window_pps;
} packet_stats_t;

void packet_stats_init(packet_stats_t *stats);
void packet_stats_on_packet(packet_stats_t *stats,
                            packet_type_t type,
                            uint8_t frame_subtype,
                            size_t packet_len,
                            bool has_rssi,
                            int8_t rssi_dbm,
                            uint64_t timestamp_us);
void packet_stats_snapshot(const packet_stats_t *stats, packet_stats_snapshot_t *out_snapshot);
void packet_stats_print(const packet_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif  // PACKET_STATS_H
