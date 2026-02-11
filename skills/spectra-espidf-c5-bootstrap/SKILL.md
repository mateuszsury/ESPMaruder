---
name: spectra-espidf-c5-bootstrap
description: Bootstrap and maintain the ESP32-C5 ESP-IDF project foundation. Use when creating project structure, tuning sdkconfig defaults, configuring build profiles, and wiring core components for later phases.
---

# SPECTRA ESP-IDF C5 Bootstrap

## Workflow

1. Create canonical component layout for core, radio engines, UI, and utilities.
2. Establish sdkconfig.defaults and profile overrides for debug, perf, and release.
3. Set compiler warnings, optimization levels, logging verbosity, and assertions.
4. Define reproducible build commands and artifact naming rules.
5. Validate clean configure, build, and flash pipeline on ESP32-C5 target.

## Deliverables

- Project skeleton with CMake targets and component boundaries.
- Baseline sdkconfig defaults documented by rationale.
- Build matrix for debug and release profiles.
- Bootstrap checklist for new contributors.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
