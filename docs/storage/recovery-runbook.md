# Storage Recovery Runbook (Phase 1.2)

## Symptoms and actions

1. NVS init failure
- Symptom: `spectra_storage_init` returns error.
- Action: run NVS erase procedure, reboot, verify `schema_ver` recreated.

2. SPIFFS mount failure
- Symptom: warning from `spiffs_mgr_init`.
- Action: check partition table label `spiffs`, verify flash health, remount.

3. SD mount failure
- Symptom: warning from `sd_card_init`.
- Action: verify card presence, wiring/pins, file system integrity.

4. Low free space on SD captures
- Symptom: `log_rotate_check` reports low space.
- Action: run cleanup (`log_rotate_cleanup`) for oldest `.pcap` files.

## Rotation policy

- Threshold: `LOG_ROTATE_MIN_FREE_MB` (default: 50 MB).
- Cleanup target: oldest `*.pcap` files first.
- SD support is optional and disabled by default for current C5 preview builds.

