# Handoff: Platform Bootstrap (Headless Variant) -> Core Engines

Date: 2026-02-11
From: agent-platform-bootstrap
To: agent-core-engines
Milestone: Phase 1.2 headless bring-up profile

## Completed

- Added `sdkconfig.headless.defaults` for no-display/no-SD bring-up.
- Extended `scripts/wsl/build_instance.sh` with `headless` profile support.
- Updated `scripts/wsl/build_instance.sh` IDF selection to prefer local ESP-IDF 5.5 installations (override via `SPECTRA_IDF_ROOT`).
- Adjusted storage init logs so disabled SD reports as expected (`SD disabled by config`).
- Updated bootstrap docs with profile usage and examples.

## Constraints

- `headless` sets `CONFIG_SPECTRA_STORAGE_ENABLE_SD=n`, so SD APIs remain compiled but mount path is skipped.
- Target remains `esp32c5` in preview mode (`idf.py --preview`) with isolated per-instance build/output paths.
- This change does not add display initialization; GUI remains stub/reserved.
- Headless profile is validated on hardware (`COM14`) with clean boot to `SPECTRA bootstrap complete` and runtime banner `Build profile: headless`.

## Recommended next step

- Continue with Phase 1.3 CLI commands, including runtime visibility of active build profile and storage health.
