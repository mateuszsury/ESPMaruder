---
name: spectra-cli-command-engine
description: Implement and evolve the command-line interface layer. Use when adding command grammar, parsing, typed arguments, dispatch routing, validation, autocomplete hints, and structured help output.
---

# SPECTRA CLI Command Engine

## Workflow

1. Define command tree grouped by module and task.
2. Implement deterministic parser with clear syntax and error reporting.
3. Validate argument types, ranges, and mutually exclusive options.
4. Map commands to engine APIs through a thin dispatch layer.
5. Keep help text synchronized with actual command capabilities.

## Deliverables

- Command grammar and module map.
- Parser and dispatcher contracts.
- Error catalog with actionable remediation messages.
- CLI reference snippets for docs and tests.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
