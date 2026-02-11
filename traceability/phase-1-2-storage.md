# Traceability - Phase 1.2 Storage Manager

Date: 2026-02-11
Phase: 1.2 Storage Manager
Scope reference: GOV-002

## Implemented artifacts

- `components/storage/CMakeLists.txt`
- `components/storage/Kconfig`
- `components/storage/include/spectra_storage.h`
- `components/storage/include/nvs_mgr.h`
- `components/storage/include/spiffs_mgr.h`
- `components/storage/include/sd_card.h`
- `components/storage/include/log_rotate.h`
- `components/storage/src/spectra_storage.c`
- `components/storage/src/nvs_mgr.c`
- `components/storage/src/spiffs_mgr.c`
- `components/storage/src/sd_card.c`
- `components/storage/src/log_rotate.c`
- `CMakeLists.txt`
- `main/CMakeLists.txt`
- `main/version.h`
- `scripts/wsl/build_instance.sh`
- `sdkconfig.defaults`
- `sdkconfig.headless.defaults`
- `docs/bootstrap/build-matrix.md`
- `docs/bootstrap/bootstrap-checklist.md`
- `docs/storage/storage-map.md`
- `docs/storage/nvs-key-catalog.md`
- `docs/storage/recovery-runbook.md`

## Test references

- TEST-STORAGE-001: WSL build verification (fresh instance).
  - Command: `./scripts/wsl/build_instance.sh wsl03 debug`
  - Result: PASS
  - Artifact: `out/build/wsl03-debug/spectra.elf`
- TEST-STORAGE-002: Config verification (fresh instance).
  - File: `out/sdkconfig/wsl03-debug.sdkconfig`
  - Check: `CONFIG_SPECTRA_STORAGE_ENABLE_SD=y`
- TEST-STORAGE-003: Existing-instance migration from legacy MP sdkconfig.
  - Command: `./scripts/wsl/build_instance.sh wsl02 debug`
  - Result: PASS (script detected MP sdkconfig and regenerated)
  - Artifact: `out/sdkconfig/wsl02-debug.sdkconfig`
  - Check: stale MP variant sdkconfig is regenerated automatically
- TEST-STORAGE-004: Headless profile build for bring-up without SD/display hardware.
  - Command: `./scripts/wsl/build_instance.sh wsl60 headless`
  - Result: PASS
  - Artifact: `out/build/wsl60-headless/spectra.elf`
  - Check: `out/sdkconfig/wsl60-headless.sdkconfig` contains `# CONFIG_SPECTRA_STORAGE_ENABLE_SD is not set` and `CONFIG_ESP32C5_REV_MAX_FULL=199`
- TEST-STORAGE-005: Windows flash and boot validation for headless profile on COM14.
  - Flash command: `python -m esptool --chip esp32c5 --port COM14 --baud 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-freq 80m --flash-size 4MB 0x2000 out/build/wsl60-headless/bootloader/bootloader.bin 0x8000 out/build/wsl60-headless/partition_table/partition-table.bin 0x10000 out/build/wsl60-headless/spectra.bin`
  - Result: PASS
  - Serial evidence: boot reaches `SPECTRA bootstrap complete`, logs `SD disabled by config`, and banner reports `Build profile: headless`

## Notes

- `CONFIG_SPECTRA_STORAGE_ENABLE_SD` defaults to `y`.
- `headless` profile explicitly disables SD (`CONFIG_SPECTRA_STORAGE_ENABLE_SD=n`) for no-SD hardware bring-up.
- Build script prefers local ESP-IDF 5.5 installations and still regenerates stale `ESP32-C5 MP` sdkconfig files from older setups.
- Runtime storage init now logs `SD disabled by config` when SD is intentionally disabled by profile.

## Documentation references

- DOC-STORAGE-001: `docs/storage/storage-map.md`
- DOC-STORAGE-002: `docs/storage/nvs-key-catalog.md`
- DOC-STORAGE-003: `docs/storage/recovery-runbook.md`
- DOC-BOOT-001: `docs/bootstrap/build-matrix.md`
- DOC-BOOT-002: `docs/bootstrap/bootstrap-checklist.md`
