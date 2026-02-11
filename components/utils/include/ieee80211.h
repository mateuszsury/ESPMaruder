#ifndef IEEE80211_H
#define IEEE80211_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IEEE80211_FC_TYPE_MGMT 0x00
#define IEEE80211_FC_TYPE_CTRL 0x01
#define IEEE80211_FC_TYPE_DATA 0x02

#define IEEE80211_FC_STYPE_ASSOC_REQ 0x00
#define IEEE80211_FC_STYPE_ASSOC_RESP 0x01
#define IEEE80211_FC_STYPE_REASSOC_REQ 0x02
#define IEEE80211_FC_STYPE_REASSOC_RESP 0x03
#define IEEE80211_FC_STYPE_PROBE_REQ 0x04
#define IEEE80211_FC_STYPE_PROBE_RESP 0x05
#define IEEE80211_FC_STYPE_BEACON 0x08
#define IEEE80211_FC_STYPE_ATIM 0x09
#define IEEE80211_FC_STYPE_DISASSOC 0x0A
#define IEEE80211_FC_STYPE_AUTH 0x0B
#define IEEE80211_FC_STYPE_DEAUTH 0x0C
#define IEEE80211_FC_STYPE_ACTION 0x0D

typedef struct __attribute__((packed)) {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t seq_ctrl;
} ieee80211_hdr_t;

typedef struct __attribute__((packed)) {
    uint64_t timestamp;
    uint16_t beacon_interval;
    uint16_t capability_info;
} ieee80211_mgmt_beacon_t;

typedef struct __attribute__((packed)) {
    uint8_t id;
    uint8_t len;
    uint8_t data[];
} ieee80211_ie_t;

typedef struct __attribute__((packed)) {
    uint16_t reason_code;
} ieee80211_deauth_t;

#ifdef __cplusplus
}
#endif

#endif  // IEEE80211_H
