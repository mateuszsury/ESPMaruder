# Handoff: Governance -> Platform Bootstrap

Date: 2026-02-11
From: agent-governance
To: agent-platform-bootstrap
Milestone: Governance gate for Sprint 0

## Completed deliverables

- `governance/sprint-0-scope-matrix.md`
- `governance/compliance-gate-checklist.md`

## Approved implementation window

Platform bootstrap may proceed only for:
- ESP-IDF project initialization and build scaffolding.
- Passive observability and storage/config interfaces.
- Safe CLI/doc legal notices.

## Constraints for next milestone

- Do not implement disruptive RF actions.
- Stay in owned/authorized lab environment scope.
- Attach `GOV-*`, `TEST-*`, and `DOC-*` references to each task/PR.

## Open risks

- `REVIEW` features in future phases require explicit written authorization before coding.
- User-facing disclaimer coverage must be verified once CLI/GUI/Web surfaces exist.
