# Utility Modules (Phase 1.5 - Step 1)

Date: 2026-02-11

## Implemented modules

- `mac_utils`
  - `mac_to_str`
  - `str_to_mac`
  - `mac_randomize` (locally administered, unicast)
  - `mac_is_broadcast`
  - `mac_is_multicast`
- `oui_lookup`
  - optional DB load from file (`oui_lookup_init`)
  - lookup by OUI prefix (`oui_lookup`)
  - builtin fallback vendors when DB is not present
- `time_utils`
  - `time_get_us` (`esp_timer`)
  - `time_get_iso8601`
  - `time_set_from_gps`
- `ieee80211.h`
  - base 802.11 headers/macros for management frame parsing
- `led_status`
  - `led_status_init`
  - `led_set_mode` (`IDLE`, `SCANNING`, `ATTACKING`, `CAPTURING`)
  - `led_status_deinit`

## Notes

- `oui_lookup` currently supports plaintext CSV-like DB lines (`AABBCC,Vendor`).
- Planned compressed OUI database loading remains pending.
- `led_status` currently uses GPIO blink patterns as hardware-agnostic fallback.
- RGB color mapping can be added when final LED hardware interface is fixed.
