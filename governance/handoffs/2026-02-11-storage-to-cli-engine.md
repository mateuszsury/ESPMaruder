# Handoff: Storage -> CLI Engine

Date: 2026-02-11
From: agent-core-engines (storage milestone)
To: agent-core-engines (CLI milestone)
Milestone: Phase 1.2 complete

## Completed

- Implemented NVS manager with schema bootstrap (`namespace: spectra`).
- Implemented SPIFFS manager (mount/read/write/list).
- Implemented SD manager API with optional compile-time enable.
- Implemented capture rotation primitives (`log_rotate_check`, `log_rotate_cleanup`).
- Integrated health reporting in `spectra_storage`.

## Constraints

- SD support is enabled by default via `CONFIG_SPECTRA_STORAGE_ENABLE_SD=y`.
- Build script prefers local ESP-IDF 5.5 installations and can be overridden with `SPECTRA_IDF_ROOT`.
- Build script auto-migrates stale per-instance sdkconfig files using legacy `ESP32-C5 MP` symbols.
- SD layer still falls back to `ESP_ERR_NOT_SUPPORTED` if a non-GPSPI profile is selected externally.
- For physical SD bring-up, verify target-specific SPI pins and card wiring.

## Recommended next step

- Implement Phase 1.3 CLI command layer and expose storage health/ops commands (`sysinfo`, `sd ls`, `set/get`).
