---
name: spectra-tft-touch-lvgl-foundation
description: Provide GUI hardware and runtime foundation for the 2.8 inch TFT touch interface. Use when initializing ILI9341, XPT2046, LVGL task loops, calibration flow, and shared SPI arbitration.
---

# SPECTRA TFT Touch LVGL Foundation

## Workflow

1. Initialize SPI bus and display pipeline with DMA-safe buffers.
2. Wire touch sampling, IRQ handling, and coordinate mapping.
3. Implement calibration flow and persist calibration in NVS.
4. Set LVGL tick, render loop, and memory alloc policy.
5. Define base UI shell: home screen, status bar, and navigation hooks.

## Deliverables

- Stable display and touch driver integration.
- Calibration routine with reset fallback path.
- LVGL runtime policy for fps and memory.
- Base GUI platform API for higher-level screens.

## Definition of Done

- Confirm outputs are reproducible and align with `plan.md` phase dependencies.
- Confirm constraints are non-disruptive and limited to authorized lab environments.
- Confirm CLI/GUI/Web and documentation references are internally consistent.
