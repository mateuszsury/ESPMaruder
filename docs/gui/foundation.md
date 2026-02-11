# GUI Foundation (Phase 1.6 - Step 1)

Date: 2026-02-11

## Implemented modules

- `gui_events`
  - Queue-based, thread-safe event transport
  - Copy-based payload model (`GUI_EVENT_MAX_DATA_LEN`)
  - Event types aligned with plan (`GUI_EVT_WIFI_*`, `GUI_EVT_BLE_*`, `GUI_EVT_*`)
- `gui_main`
  - GUI lifecycle API (`gui_init`, `gui_start`, `gui_stop`, `gui_deinit`)
  - Task skeleton and queue consumption loop
  - Navigation/toast placeholders for future LVGL integration
  - Mutex API (`gui_lock`/`gui_unlock`)
- `gui_theme`
  - Dark/light palette definitions
  - Runtime theme selector
- `tft_driver` and `touch_driver`
  - Headless-safe stubs for now
  - Hardware path reserved for next 1.6 iteration

## Headless behavior

- `headless` profile sets:
  - `CONFIG_SPECTRA_GUI_ENABLE=n`
  - `CONFIG_SPECTRA_GUI_ENABLE_HW_DRIVERS=n`
- `gui_init()` is still called from `app_main()`, but resolves to no-op path in headless mode.

## Next step (1.6 step 2)

- Replace stub TFT/touch layer with real SPI + DMA ILI9341/XPT2046 drivers.
- Add LVGL init, display buffers in internal SRAM (`SPECTRA_MALLOC_DMA`), and flush/input callbacks.
