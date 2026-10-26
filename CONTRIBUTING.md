# Contributing

Thanks for contributing to SPECTRA.

## Ground rules

- Keep all work in authorized lab scope and defensive context.
- Preserve traceability: each milestone should map code -> test evidence -> docs.
- Add/update governance handoff notes for completed milestones.
- Do not add active offensive behavior outside explicitly approved scope.

## Development workflow

1. Create a branch from `main`.
2. Implement a small, reviewable change.
3. Validate with at least one build profile (prefer `headless`).
4. Update docs and traceability when behavior changes.
5. Open a pull request using the PR template.

## Commit guidance

- Use clear commit subjects in imperative mood.
- Keep commits focused (one concern per commit).
- Include phase references where relevant (example: `phase-2.1`).

## Local validation

```bash
./scripts/wsl/build_instance.sh dev01 headless
```

## Pull request checklist

- Build passes locally.
- Docs updated (`docs/` and/or README).
- Traceability updated (`traceability/`).
- Governance gate note added when closing a milestone (`governance/gate-runs/`).
- Handoff note added for next agent (`governance/handoffs/`).

## Authorship and contributions

Project-maintained changes use the Git identity `Mateusz Sury
<66475105+mateuszsury@users.noreply.github.com>`. Configure it locally before
committing:

```sh
git config user.name "Mateusz Sury"
git config user.email "66475105+mateuszsury@users.noreply.github.com"
git config user.useConfigOnly true
```

The maintainer takes responsibility for tool-assisted changes. Do not add AI
tools as authors or co-authors. Preserve genuine human contributions, upstream
history, copyright notices and third-party licenses. Keep release provenance
and checksums intact. Describe the purpose of each change and record relevant
validation; avoid generic upload messages and environment-specific worktree names.
