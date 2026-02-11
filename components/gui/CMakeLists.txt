idf_component_register(
    SRCS
        "src/gui_events.c"
        "src/gui_main.c"
        "src/gui_theme.c"
        "src/tft_driver.c"
        "src/touch_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_timer
        freertos
        log
)
