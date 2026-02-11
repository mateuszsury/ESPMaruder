# Packet Buffer (Phase 1.4 - Step 1)

Date: 2026-02-11

## Scope

`packet_buffer` provides an SPSC packet queue for captured frames before filtering/export.

## Current behavior

- PSRAM-first allocation (`MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT`).
- Internal SRAM fallback with capped capacity (`8 KB`) if PSRAM allocation is unavailable.
- Variable-length packet records with metadata:
  - `type` (`PKT_TYPE_WIFI`, `PKT_TYPE_BLE`, `PKT_TYPE_802154`)
  - `timestamp_us`
  - `length`
- API:
  - `packet_buffer_init`
  - `packet_buffer_push`
  - `packet_buffer_pop`
  - `packet_buffer_flush`
  - `packet_buffer_stats`
  - `packet_buffer_deinit`

## Limits

- Default capacity: `32 KB` if requested size is `0`.
- Max frame size: `2048` bytes.

## Notes

- Buffer is designed for single producer / single consumer usage.
- Corruption guards are present; inconsistent queue state triggers queue flush.
