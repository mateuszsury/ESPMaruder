# SPECTRA Bootstrap Checklist (New Contributor)

## Environment

- Install ESP-IDF v5.5+ with ESP32-C5 target support.
- Open an ESP-IDF shell where `idf.py` and `IDF_PATH` are available.
- Confirm USB permissions and serial access to the board.

## Bootstrap Steps

1. Validate governance scope in `governance/sprint-0-scope-matrix.md` (`GOV-001` only).
2. Configure target:
   - `idf.py set-target esp32c5`
3. Configure build profile:
   - debug: `sdkconfig.defaults;sdkconfig.debug.defaults`
   - release: `sdkconfig.defaults;sdkconfig.release.defaults`
   - headless (no display/SD hardware): `sdkconfig.defaults;sdkconfig.headless.defaults`
4. Build and check for warnings/errors.
5. Flash and monitor over serial.
6. Verify startup banner and legal notice output.

## Acceptance for phase 1.1

- Project configures without manual patching.
- Build completes using at least one profile.
- Firmware boots and prints version + legal disclaimer.
- Component boundaries exist for `core_system`, `storage`, and `cli`.
