---
name: spectra-gps-wardriving-telemetry
description: Integrate GPS telemetry with scan results for controlled field surveys. Use when implementing NMEA parsing, timestamp alignment, route storage, map export, and privacy-preserving telemetry controls.
---

# SPECTRA GPS and Geotag Telemetry

## Workflow

1. Ingest GPS stream and validate fix quality before tagging observations.
2. Align radio events and position samples on a consistent time base.
3. Store geotagged telemetry with retention and privacy controls.
4. Export route and scan overlays in open formats.
5. Require explicit opt-in for location data capture.

## Deliverables

- GPS parser and quality gating rules.
- Geotagged telemetry schema.
- Route export format and sample artifacts.
- Privacy defaults for location capture.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
