---
name: spectra-wifi-observability
description: Implement passive Wi-Fi observability on 2.4 and 5 GHz for defensive analysis. Use when building scan, monitor, metadata parsing, channel occupancy, and capture export features without disruptive actions.
---

# SPECTRA Wi-Fi Observability

## Workflow

1. Implement active and passive AP discovery with normalized metadata.
2. Collect passive monitor traffic and compute protocol-level counters.
3. Track channel occupancy and signal quality over time windows.
4. Capture EAPOL and PMKID events for defensive audit context only.
5. Reject implementation of deauth, flooding, spoofing, or evil twin behaviors.

## Deliverables

- Wi-Fi scan API and telemetry model.
- Passive sniffer metrics and event streams.
- Channel analytics snapshots for GUI and web.
- Compliance note documenting prohibited active attacks.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
