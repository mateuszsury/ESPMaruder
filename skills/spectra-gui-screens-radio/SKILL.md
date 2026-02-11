---
name: spectra-gui-screens-radio
description: Design and implement LVGL screens for radio observability modules. Use when creating list/detail screens, live counters, filters, navigation transitions, and event summaries across Wi-Fi, BLE, Zigbee, and Thread.
---

# SPECTRA GUI Screens for Radio Modules

## Workflow

1. Define screen contracts with required state and engine dependencies.
2. Implement list, detail, and live telemetry widgets per radio module.
3. Apply reusable interaction patterns for navigation and dialogs.
4. Handle empty, loading, and degraded memory states explicitly.
5. Align labels and actions with passive observability scope.

## Deliverables

- Screen inventory and navigation graph.
- Widget contracts for radio data binding.
- UX rules for errors and degraded modes.
- Localization-ready label key list.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
