# SPECTRA Sprint 0 - Scope Matrix and Approved Scope

## Approved scope statement (Sprint 0)

Sprint 0 is limited to controlled lab setup and defensive platform bootstrap:
- ESP-IDF project initialization for ESP32-C5.
- Build system and component scaffolding.
- Passive observability foundations (logging, metrics hooks, storage interfaces).
- Compliance disclaimers in docs and operator-facing interfaces.

Only owned devices and authorized lab networks are in scope.
Any task that can disrupt, spoof, flood, jam, or access non-owned systems is out of scope.

## Scope matrix

| ID | Category | Status | Reason | Phase link (`plan.md`) |
|---|---|---|---|---|
| GOV-001 | Project bootstrap (`idf.py`, CMake, component layout) | ALLOW | Required base setup | 1.1 |
| GOV-002 | Storage and config skeleton (NVS/SD interfaces) | ALLOW | Defensive telemetry storage only | 1.2 |
| GOV-003 | CLI help, legal notice, safe command guardrails | ALLOW | Informs operator and prevents misuse | 1.3 |
| GOV-004 | Packet parsing interfaces without active injection | ALLOW | Passive observability | 1.4 |
| GOV-005 | Passive scan and decoding modules | REVIEW | Allowed only for owned/authorized lab devices | 2.x, 3.x, 4.x |
| GOV-006 | RF transmission features (deauth, spoofing, flooding, jamming, replay, rogue AP) | DENY | Disruptive behavior and legal risk | 2.3, 2.4, 3.3, 4.3, 4.4 |
| GOV-007 | Any non-lab target or unclear authorization | DENY | Unauthorized scope | All |

## Traceability

Each implementation PR should map to one scope ID from this document and include:
- test evidence reference (`TEST-*`)
- documentation reference (`DOC-*`)
- compliance decision note (`GOV-*`)

## Gate result

Governance gate for Sprint 0: PASS (conditional)
- Condition 1: only `ALLOW` items can be implemented directly.
- Condition 2: `REVIEW` items require written authorization note before coding.
- Condition 3: `DENY` items are blocked.
