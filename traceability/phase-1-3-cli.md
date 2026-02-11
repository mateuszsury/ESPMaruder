# Traceability - Phase 1.3 CLI Engine

Date: 2026-02-11
Phase: 1.3 CLI Engine
Scope reference: GOV-002

## Implemented artifacts

- `components/cli/CMakeLists.txt`
- `components/cli/include/cli_colors.h`
- `components/cli/include/cli_engine.h`
- `components/cli/src/cli_engine.c`
- `components/cli/src/spectra_cli.c`
- `sdkconfig.defaults`
- `docs/cli/commands.md`

## Test references

- TEST-CLI-001: WSL build verification (`headless` profile).
  - Command: `./scripts/wsl/build_instance.sh wsl60 headless`
  - Result: PASS
  - Artifact: `out/build/wsl60-headless/spectra.elf`
- TEST-CLI-002: Windows flash verification on `COM14`.
  - Command: `python -m esptool --chip esp32c5 --port COM14 --baud 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-freq 80m --flash-size 4MB 0x2000 out/build/wsl60-headless/bootloader/bootloader.bin 0x8000 out/build/wsl60-headless/partition_table/partition-table.bin 0x10000 out/build/wsl60-headless/spectra.bin`
  - Result: PASS
- TEST-CLI-003: Runtime command validation on device (`COM14`).
  - Commands: `version`, `sysinfo`, `set test_key test_val`, `get test_key`, `sd ls /`
  - Result: PASS
  - Evidence:
    - `version` shows `build_profile: headless`
    - `set/get` roundtrip succeeds (`test_key=test_val`)
    - `sd ls /` returns `sd not mounted` in headless profile
- TEST-CLI-004: Stability check after CLI init.
  - Check: no reboot loop and no `Stack protection fault` after replacing REPL path with dedicated UART CLI task.
  - Result: PASS

## Notes

- CLI runs as dedicated FreeRTOS task (`priority 5`) over UART.
- Prompt is colorized (`spectra>`) and basic line editing supports backspace.
- History ring buffer (16 entries) is maintained in-memory for phase 1.3 baseline.
- Build profile macro is propagated to CLI so `version` reflects active profile.

## Documentation references

- DOC-CLI-001: `docs/cli/commands.md`
