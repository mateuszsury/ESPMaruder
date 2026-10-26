# SPECTRA Agents

## Purpose
Coordinate specialized agents for phased implementation of the ESP32-C5 SPECTRA platform from `plan.md`.

## Global Rules
- Keep all work in controlled lab context and authorized scope.
- Prefer passive observability and defensive validation workflows.
- Track deliverables by phase and maintain traceability to tests and docs.
- Require explicit handoff notes between agents after each milestone.

## Recommended Execution Order
1. `agents/agent-governance.md`
2. `agents/agent-platform-bootstrap.md`
3. `agents/agent-core-engines.md`
4. `agents/agent-radio-observability.md`
5. `agents/agent-gui-web.md`
6. `agents/agent-hardening-test.md`
7. `agents/agent-docs-release.md`

## Maintainer identity

Follow the authorship policy in CONTRIBUTING.md. Maintainer-owned commits use
`Mateusz Sury <66475105+mateuszsury@users.noreply.github.com>`.
Do not add AI author/co-author or generated-by attribution. Preserve genuine
upstream/human/bot commit authors, copyright notices and licenses. Configure Git
locally before committing; do not rewrite release tags or published checksums.
