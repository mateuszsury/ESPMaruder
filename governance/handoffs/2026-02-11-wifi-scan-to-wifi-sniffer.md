# Handoff: Wi-Fi Scan Baseline -> Wi-Fi Sniffer

Date: 2026-02-11
From: agent-radio-observability (phase 2.1 step 1)
To: agent-radio-observability (phase 2.2)
Milestone: Wi-Fi engine and AP scanning baseline complete

## Completed

- Implemented Wi-Fi lifecycle engine with init/start/stop/deinit controls.
- Implemented AP scan API with sorted results and compact result model.
- Added PSRAM-first allocation for scan result arrays with fallback.
- Updated docs, traceability, and gate artifacts.
- Verified clean `headless` build (`wsl63`).

## Constraints

- No station scan yet.
- No channel occupancy scan yet.
- No promiscuous sniffing or packet pipeline integration yet.

## Recommended next step

- Implement phase 2.2 sniffer path:
  - promiscuous callback + non-blocking queue transfer
  - filter modes and optional channel hopping
  - integration with packet engine buffer/stats.
