---
name: spectra-storage-config-core
description: Design and maintain storage architecture for configuration, telemetry, and capture files. Use when implementing NVS keys, SPIFFS assets, SD card logging, retention, and crash-safe write behavior.
---

# SPECTRA Storage Config Core

## Workflow

1. Define ownership of data domains: config, runtime cache, logs, and captures.
2. Implement schema and versioning strategy for NVS namespaces.
3. Set retention and rotation for logs and PCAP files on SD.
4. Handle mount, unmount, corruption, and low-space recovery paths.
5. Expose storage health metrics for CLI and GUI status views.

## Deliverables

- Storage map with quotas and lifecycle rules.
- NVS key catalog with migration plan.
- Log and capture rotation policy.
- Recovery runbook for storage faults.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
