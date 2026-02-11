# SPECTRA Storage Map (Phase 1.2)

## Domains and ownership

| Domain | Backend | Purpose | Lifecycle |
|---|---|---|---|
| Config | NVS (`namespace: spectra`) | persistent app settings and calibration | survives reboot/update |
| Runtime cache | SPIFFS (`/spiffs`) | small local assets and temporary structured files | persistent, managed by app |
| Captures/logs | SD (`/sdcard`, optional) | packet captures and larger logs | removable media, rotation policy |

## Quotas and defaults

- NVS: schema versioned (`schema_ver`) with migration-ready API.
- SPIFFS: mounted on partition label `spiffs`, `max_files=8`.
- SD: enabled by default (`CONFIG_SPECTRA_STORAGE_ENABLE_SD=y`), with backend fallback when GPSPI is unavailable on target profile.

## Initialization policy

1. NVS is mandatory (`spectra_storage_init` fails if NVS init fails).
2. SPIFFS is best-effort (warning on failure, app continues).
3. SD is optional (warning on failure or disabled mode, app continues).
