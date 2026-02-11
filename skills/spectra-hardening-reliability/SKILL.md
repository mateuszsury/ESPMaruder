---
name: spectra-hardening-reliability
description: Harden runtime reliability across memory, tasks, and error handling paths. Use when implementing watchdog strategy, heap policies, graceful degradation, crash diagnostics, and restart recovery behavior.
---

# SPECTRA Hardening and Reliability

## Workflow

1. Define memory budgets for SRAM and PSRAM by subsystem.
2. Enable watchdog coverage for critical tasks and loops.
3. Implement progressive degradation under low-memory conditions.
4. Capture crash context and enforce deterministic recovery flow.
5. Stress-test stability under sustained telemetry load.

## Deliverables

- Memory budget and degradation policy.
- Watchdog and health monitoring plan.
- Crash dump and reboot diagnostics checklist.
- Long-run reliability acceptance criteria.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
