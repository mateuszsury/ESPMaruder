#include "packet_filter.h"

#include <string.h>

static bool mac_equals(const uint8_t a[6], const uint8_t b[6]) {
    return memcmp(a, b, 6) == 0;
}

void packet_filter_init(packet_filter_t *filter) {
    if (filter == NULL) {
        return;
    }

    memset(filter, 0, sizeof(*filter));
    filter->frame_type_mask = 0xFFU;
    filter->frame_subtype_mask = 0xFFFFU;
    filter->rssi_min = -127;
}

bool packet_filter_match(const packet_filter_t *filter, const packet_filter_meta_t *packet) {
    if (filter == NULL || packet == NULL) {
        return false;
    }

    if (packet->frame_type >= 8) {
        return false;
    }
    if (packet->frame_subtype >= 16) {
        return false;
    }

    const uint8_t type_bit = (uint8_t)(1U << packet->frame_type);
    if ((filter->frame_type_mask & type_bit) == 0U) {
        return false;
    }

    const uint16_t subtype_bit = (uint16_t)(1U << packet->frame_subtype);
    if ((filter->frame_subtype_mask & subtype_bit) == 0U) {
        return false;
    }

    if (filter->src_mac_enabled && !mac_equals(filter->src_mac, packet->src_mac)) {
        return false;
    }
    if (filter->dst_mac_enabled && !mac_equals(filter->dst_mac, packet->dst_mac)) {
        return false;
    }
    if (filter->bssid_enabled && !mac_equals(filter->bssid, packet->bssid)) {
        return false;
    }

    if (filter->rssi_enabled) {
        if (!packet->has_rssi) {
            return false;
        }
        if (packet->rssi < filter->rssi_min) {
            return false;
        }
    }

    return true;
}
