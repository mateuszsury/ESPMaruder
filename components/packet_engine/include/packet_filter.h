#ifndef PACKET_FILTER_H
#define PACKET_FILTER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t frame_type_mask;
    uint16_t frame_subtype_mask;
    uint8_t src_mac[6];
    uint8_t dst_mac[6];
    uint8_t bssid[6];
    int8_t rssi_min;
    bool src_mac_enabled;
    bool dst_mac_enabled;
    bool bssid_enabled;
    bool rssi_enabled;
} packet_filter_t;

typedef struct {
    uint8_t frame_type;
    uint8_t frame_subtype;
    uint8_t src_mac[6];
    uint8_t dst_mac[6];
    uint8_t bssid[6];
    int8_t rssi;
    bool has_rssi;
} packet_filter_meta_t;

void packet_filter_init(packet_filter_t *filter);
bool packet_filter_match(const packet_filter_t *filter, const packet_filter_meta_t *packet);

#ifdef __cplusplus
}
#endif

#endif  // PACKET_FILTER_H
