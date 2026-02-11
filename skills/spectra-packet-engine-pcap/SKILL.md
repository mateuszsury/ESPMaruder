---
name: spectra-packet-engine-pcap
description: Build and optimize the packet pipeline shared by radio modules. Use when normalizing packet metadata, applying filters, buffering safely, writing PCAP output, and validating analyzer compatibility.
---

# SPECTRA Packet Engine PCAP

## Workflow

1. Define canonical packet header with timestamp and source radio metadata.
2. Implement filter chain for type, channel, address, and protocol criteria.
3. Manage ring buffers with backpressure and drop accounting.
4. Write PCAP records with strict format correctness.
5. Verify output with Wireshark and regression fixtures.

## Deliverables

- Packet schema and filter contract.
- PCAP writer with deterministic tests.
- Capture performance metrics and drop statistics.
- Compatibility checklist for offline analyzers.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
