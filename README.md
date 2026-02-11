# SPECTRA for ESP32-C5

[![CI](https://github.com/mateuszsury/ESPMaruder/actions/workflows/ci.yml/badge.svg)](https://github.com/mateuszsury/ESPMaruder/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

SPECTRA is an ESP-IDF based platform for controlled-lab radio observability and defensive validation on ESP32-C5.

This repository tracks implementation from `plan.md` with explicit phase traceability, governance gates, and milestone handoffs.

## Legal and scope notice

Use only in authorized, controlled laboratory environments on owned devices and networks.

- Full notice: `docs/LEGAL_NOTICE.md`
- Governance scope: `governance/sprint-0-scope-matrix.md`

## Current status

Implemented baseline milestones:

- Phase 1.1: Bootstrap and project skeleton
- Phase 1.2: Storage manager baseline
- Phase 1.3: CLI baseline
- Phase 1.4: Packet engine baseline
- Phase 1.5: Utility modules baseline
- Phase 1.6 (step 1): GUI foundation (headless-safe)
- Phase 2.1 (step 1): Wi-Fi engine + AP scanner baseline

Traceability records live in `traceability/`.

## Repository layout

```text
components/     ESP-IDF components (core, storage, cli, packet, gui, radio, web, utils)
main/           app entrypoint and firmware metadata
docs/           technical docs and runbooks
governance/     compliance gates and inter-agent handoffs
traceability/   implementation-to-test mapping by phase
scripts/        build helper scripts
agents/         agent role contracts
skills/         local skill bundles used by the workflow
```

## Prerequisites

- ESP-IDF v5.5+ with ESP32-C5 support
- `idf.py` available in shell
- Python and toolchain from ESP-IDF install

## Build

Preferred isolated build (supports parallel local instances):

```bash
./scripts/wsl/build_instance.sh wsl01 headless
./scripts/wsl/build_instance.sh wsl02 debug
./scripts/wsl/build_instance.sh wsl03 release
```

Profiles:

- `headless`: bring-up without display/SD hardware
- `debug`: local diagnostics
- `release`: optimized artifact

## Flash

```bash
idf.py -p COM14 flash monitor
```

Or use generated artifacts from `out/build/<instance>-<profile>/`.

## Documentation index

- Main docs index: `docs/README.md`
- Build matrix: `docs/bootstrap/build-matrix.md`
- Wi-Fi baseline: `docs/wifi/wifi-engine-scanner.md`
- GUI foundation: `docs/gui/foundation.md`
- Packet engine docs: `docs/packet-engine/`
- Storage docs: `docs/storage/`

## Contributing

Please read:

- `CONTRIBUTING.md`
- `SECURITY.md`
- `CODE_OF_CONDUCT.md`
- `SUPPORT.md`

## License

MIT License. See `LICENSE`.
