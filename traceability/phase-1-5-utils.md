# Traceability - Phase 1.5 Utility Modules

Date: 2026-02-11
Phase: 1.5 Utility modules
Scope reference: GOV-002

## Implemented artifacts

- `components/utils/include/mac_utils.h`
- `components/utils/src/mac_utils.c`
- `components/utils/include/oui_lookup.h`
- `components/utils/src/oui_lookup.c`
- `components/utils/include/time_utils.h`
- `components/utils/src/time_utils.c`
- `components/utils/include/ieee80211.h`
- `components/utils/include/led_status.h`
- `components/utils/src/led_status.c`
- `components/utils/CMakeLists.txt`
- `components/utils/README.md`
- `docs/utils/core-utils.md`

## Test references

- TEST-UTILS-001: Headless build verification.
  - Command: `./scripts/wsl/build_instance.sh wsl61 headless`
  - Result: PASS
  - Evidence: `esp-idf/utils/libutils.a` includes `led_status.c` and image generation succeeds.
- TEST-UTILS-002: Artifact generation.
  - Artifact: `out/build/wsl61-headless/spectra.bin`
  - Result: PASS

## Notes

- `led_status` currently uses GPIO blink patterns as a hardware-safe fallback.
- RGB color mapping from the phase plan will be finalized once the target LED hardware interface is fixed.
- OUI compressed database loading is still pending; current implementation supports optional file load plus builtin fallbacks.

## Documentation references

- DOC-UTILS-001: `docs/utils/core-utils.md`
