# Packet Stats (Phase 1.4 - Step 4)

Date: 2026-02-11

## Scope

`packet_stats` aggregates packet telemetry for CLI/GUI reporting.

## Current behavior

- Counters:
  - total packets
  - total bytes
  - per packet type
  - per subtype (`0..15`)
- RSSI histogram:
  - range `-120..0 dBm`
  - bin width `5 dBm`
- Throughput metric:
  - packets-per-second over sliding 1-second window (`last_window_pps`)
- Printable summary:
  - `packet_stats_print(...)`

## API

- `packet_stats_init(...)`
- `packet_stats_on_packet(...)`
- `packet_stats_snapshot(...)`
- `packet_stats_print(...)`

## Notes

- Stats module is standalone and ready for wiring into packet pipeline and CLI commands.
