# Wi-Fi Engine + Scanner Baseline (Phase 2.1 - Step 1)

Date: 2026-02-11

## Implemented scope

- `wifi_engine`
  - Netif/event loop safe init (`esp_netif_init`, `esp_event_loop_create_default`)
  - Wi-Fi init and baseline config (`WIFI_MODE_STA`, `WIFI_STORAGE_RAM`)
  - Start/stop/deinit lifecycle
  - Band, channel, and MAC setter/getter API
- `wifi_scanner`
  - AP scanning API (`wifi_scan_ap`)
  - Blocking scan flow based on `esp_wifi_scan_start(..., true)`
  - RSSI-descending sort
  - Result projection to compact `wifi_ap_info_t`
  - PSRAM-first allocation with internal RAM fallback

## Current constraints

- This step covers passive AP scan only.
- Station scan and channel occupancy scan are not implemented yet.
- Sniffer/promiscuous mode path is deferred to Phase 2.2.

## Safety posture

- No active attack functionality was added in this milestone.
- Implementation remains within observability-only baseline suitable for controlled lab use.
