---
name: spectra-governance-compliance
description: Define legal and ethical boundaries for the SPECTRA platform. Use when planning features, validating scope, writing disclaimers, reviewing risk, or approving release artifacts that touch security testing behavior.
---

# SPECTRA Governance and Compliance

## Workflow

1. Define allowed test scope: owned devices, written consent, controlled lab network.
2. Classify each planned feature as passive observability, defensive validation, or prohibited offensive action.
3. Require explicit legal notice in CLI, GUI, web UI, and project documentation.
4. Create a traceable decision log for high-risk capabilities and rejected items.
5. Block implementation tasks that imply disruption, spoofing, flooding, or unauthorized access.

## Deliverables

- Scope matrix with allow, review, and deny categories.
- Compliance checklist tied to release gates.
- Standard disclaimer text in Polish and English.
- Issue template for legal and ethical review.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
