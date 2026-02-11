# gui

Phase 1.6 foundation status:

- Implemented: GUI event queue (`gui_events`) with thread-safe copy-based payloads.
- Implemented: GUI task skeleton (`gui_main`) with lifecycle API.
- Implemented: theme baseline (`gui_theme`) with dark/light palettes.
- Implemented: `tft_driver` and `touch_driver` headless-safe stubs.

Notes:

- In `headless` profile, GUI hardware path should be disabled (`CONFIG_SPECTRA_GUI_ENABLE=n`).
- LVGL integration and real TFT/Touch SPI DMA drivers are planned in the next 1.6 iterations.
