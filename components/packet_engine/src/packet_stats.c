#include "packet_stats.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static size_t rssi_to_bin(int8_t rssi_dbm) {
    int value = rssi_dbm;
    if (value < PACKET_STATS_RSSI_MIN_DBM) {
        value = PACKET_STATS_RSSI_MIN_DBM;
    }
    if (value > PACKET_STATS_RSSI_MAX_DBM) {
        value = PACKET_STATS_RSSI_MAX_DBM;
    }

    const int offset = value - PACKET_STATS_RSSI_MIN_DBM;
    size_t bin = (size_t)(offset / PACKET_STATS_RSSI_BIN_DBM);
    if (bin >= PACKET_STATS_RSSI_BIN_COUNT) {
        bin = PACKET_STATS_RSSI_BIN_COUNT - 1;
    }
    return bin;
}

void packet_stats_init(packet_stats_t *stats) {
    if (stats == NULL) {
        return;
    }
    memset(stats, 0, sizeof(*stats));
}

void packet_stats_on_packet(packet_stats_t *stats,
                            packet_type_t type,
                            uint8_t frame_subtype,
                            size_t packet_len,
                            bool has_rssi,
                            int8_t rssi_dbm,
                            uint64_t timestamp_us) {
    if (stats == NULL) {
        return;
    }

    stats->total_packets++;
    stats->total_bytes += packet_len;

    if ((unsigned)type < 4U) {
        stats->packets_per_type[(unsigned)type]++;
    }
    if (frame_subtype < PACKET_STATS_MAX_SUBTYPE) {
        stats->packets_per_subtype[frame_subtype]++;
    }
    if (has_rssi) {
        const size_t bin = rssi_to_bin(rssi_dbm);
        stats->rssi_histogram[bin]++;
    }

    if (stats->window_start_us == 0) {
        stats->window_start_us = timestamp_us;
    }

    if (timestamp_us >= stats->window_start_us + 1000000ULL) {
        const uint64_t elapsed = timestamp_us - stats->window_start_us;
        const uint64_t whole_seconds = elapsed / 1000000ULL;

        stats->last_window_pps = stats->window_packet_count;
        stats->window_packet_count = 0;
        stats->window_start_us += whole_seconds * 1000000ULL;
    }

    stats->window_packet_count++;
}

void packet_stats_snapshot(const packet_stats_t *stats, packet_stats_snapshot_t *out_snapshot) {
    if (stats == NULL || out_snapshot == NULL) {
        return;
    }

    memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->total_packets = stats->total_packets;
    out_snapshot->total_bytes = stats->total_bytes;
    memcpy(out_snapshot->packets_per_type, stats->packets_per_type, sizeof(out_snapshot->packets_per_type));
    memcpy(out_snapshot->packets_per_subtype, stats->packets_per_subtype, sizeof(out_snapshot->packets_per_subtype));
    memcpy(out_snapshot->rssi_histogram, stats->rssi_histogram, sizeof(out_snapshot->rssi_histogram));
    out_snapshot->last_window_pps = stats->last_window_pps;
}

void packet_stats_print(const packet_stats_t *stats) {
    if (stats == NULL) {
        return;
    }

    printf("packet_stats:\n");
    printf("  total_packets: %" PRIu64 "\n", stats->total_packets);
    printf("  total_bytes: %" PRIu64 "\n", stats->total_bytes);
    printf("  pps_last_window: %" PRIu32 "\n", stats->last_window_pps);
    printf("  type_wifi: %" PRIu64 "\n", stats->packets_per_type[PKT_TYPE_WIFI]);
    printf("  type_ble: %" PRIu64 "\n", stats->packets_per_type[PKT_TYPE_BLE]);
    printf("  type_802154: %" PRIu64 "\n", stats->packets_per_type[PKT_TYPE_802154]);

    printf("  subtype_counts:\n");
    for (size_t i = 0; i < PACKET_STATS_MAX_SUBTYPE; i++) {
        if (stats->packets_per_subtype[i] > 0) {
            printf("    [%u] = %" PRIu64 "\n", (unsigned)i, stats->packets_per_subtype[i]);
        }
    }

    printf("  rssi_histogram_dbm_%d_step_%d:\n", PACKET_STATS_RSSI_MIN_DBM, PACKET_STATS_RSSI_BIN_DBM);
    for (size_t i = 0; i < PACKET_STATS_RSSI_BIN_COUNT; i++) {
        const int low = PACKET_STATS_RSSI_MIN_DBM + ((int)i * PACKET_STATS_RSSI_BIN_DBM);
        const int high = low + PACKET_STATS_RSSI_BIN_DBM - 1;
        printf("    [%d..%d] = %" PRIu64 "\n", low, high, stats->rssi_histogram[i]);
    }
}
