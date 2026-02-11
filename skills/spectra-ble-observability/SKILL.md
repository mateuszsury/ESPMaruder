---
name: spectra-ble-observability
description: Implement passive Bluetooth LE observability and diagnostics. Use when building advertising scans, profile parsing, service discovery, and telemetry export for authorized devices in controlled tests.
---

# SPECTRA BLE Observability

## Workflow

1. Collect advertisements with normalized address and data structures.
2. Decode manufacturer data, iBeacon, and Eddystone payloads.
3. Implement authorized GATT discovery and read diagnostics.
4. Track connection parameters and pairing metadata for audit.
5. Reject spam, replay, spoofing, and denial-of-service feature requests.

## Deliverables

- BLE scanner with parsed telemetry fields.
- Device profile model and GATT explorer contracts.
- Export format for offline analysis.
- Guardrail note for prohibited offensive BLE actions.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
