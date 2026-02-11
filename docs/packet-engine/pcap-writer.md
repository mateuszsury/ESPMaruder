# PCAP Writer (Phase 1.4 - Step 2)

Date: 2026-02-11

## Scope

`pcap_writer` persists captured frames to standard libpcap files.

## Current behavior

- Writes global libpcap header (`magic 0xa1b2c3d4`, version 2.4).
- Supports link types:
  - `DLT_IEEE802_11` (`105`)
  - `DLT_IEEE802_11_RADIO` (`127`)
  - `DLT_IEEE802_15_4` (`195`)
  - `DLT_BLUETOOTH_LE_LL` (`251`)
- Writes per-packet record header with `timestamp_us`.
- Enforces `snaplen` truncation for on-disk payload.
- Auto-splits files when threshold is exceeded (default `50 MB`).

## API

- `pcap_writer_open(...)`
- `pcap_writer_open_ex(...)`
- `pcap_writer_write_packet(...)`
- `pcap_writer_close(...)`

## Split naming

- Base file: unchanged path (split index `0`).
- Rotated files: `<base>_NNNN<ext>`, e.g. `capture_0001.pcap`.

## Notes

- Writer is currently a standalone module; pipeline hookup to live capture is pending.
