---
name: spectra-802154-zigbee-thread-observability
description: Implement IEEE 802.15.4 observability for Zigbee and Thread in non-disruptive mode. Use when building channel scans, frame decoding, topology mapping, and passive telemetry export.
---

# SPECTRA 802.15.4 Zigbee and Thread Observability

## Workflow

1. Implement channel discovery for PANs, leaders, routers, and endpoints.
2. Decode frame layers into normalized analytics events.
3. Map topology and role relationships over time.
4. Export captures with metadata for offline inspection.
5. Reject injection, jamming, forced rejoin, or impersonation requests.

## Deliverables

- Discovery and topology API for Zigbee and Thread.
- Packet decoder contracts and test vectors.
- Passive capture export integration.
- Explicit prohibition of active disruption features.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
