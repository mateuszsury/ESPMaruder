# Packet Filter (Phase 1.4 - Step 3)

Date: 2026-02-11

## Scope

`packet_filter` provides deterministic packet admission logic before export/analysis.

## Current behavior

- Filter fields:
  - `frame_type_mask`
  - `frame_subtype_mask`
  - `src_mac`, `dst_mac`, `bssid` (+ enable flags)
  - `rssi_min` (+ enable flag)
- Match function:
  - rejects out-of-range frame type/subtype values
  - applies type/subtype masks
  - applies optional MAC/BSSID exact matching
  - applies optional RSSI threshold

## API

- `packet_filter_init(...)` (defaults: allow all type/subtype, disabled MAC/RSSI constraints)
- `packet_filter_match(...)`

## Notes

- Module is transport-agnostic and operates on packet metadata only.
- Integration with producer pipeline is pending.
