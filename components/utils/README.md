# utils

Phase 1.5 bootstrap status:

- Implemented: `mac_utils` (`mac_to_str`, `str_to_mac`, randomize, broadcast/multicast checks).
- Implemented: `oui_lookup` (optional DB load + builtin fallback lookup).
- Implemented: `time_utils` (`time_get_us`, `time_get_iso8601`, `time_set_from_gps`).
- Implemented: `ieee80211.h` core 802.11 structures and subtype/type macros.
- Implemented: `led_status` (`led_set_mode` patterns for idle/scanning/attacking/capturing).
