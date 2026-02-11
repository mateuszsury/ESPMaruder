# packet_engine

Phase 1.4 bootstrap status:

- Implemented: `packet_buffer` (SPSC ring buffer, PSRAM-first allocation with internal SRAM fallback, queue stats).
- Implemented: `pcap_writer` (libpcap global header, packet write, auto-split by file size).
- Implemented: `packet_filter` (mask/type/subtype + MAC/BSSID + RSSI matching).
- Implemented: `packet_stats` (counters, RSSI histogram, pps window, printable summary).
