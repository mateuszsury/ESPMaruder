# Handoff: Platform Bootstrap -> Core Engines

Date: 2026-02-11
From: agent-platform-bootstrap
To: agent-core-engines
Milestone: Phase 1.1 bootstrap skeleton

## Completed

- ESP-IDF project scaffold for ESP32-C5 with custom partition table.
- Baseline and profile-specific sdkconfig defaults.
- Boot flow with NVS/event loop init and startup legal notice.
- Component boundaries and stubs:
  - `core_system`
  - `storage`
  - `cli`

## Constraints carried forward

- Governance scope enforced (`GOV-001`, `GOV-002`, `GOV-003`, `GOV-004` only).
- No disruptive RF behavior implementation.
- Lab-only authorized usage wording must remain in user-facing output and docs.

## Recommended next tasks

1. Implement `components/storage` APIs and NVS schema baseline (Phase 1.2).
2. Implement CLI parser/dispatcher contracts in `components/cli` (Phase 1.3).
3. Add packet engine core ring buffer and PCAP writer stubs (Phase 1.4).

