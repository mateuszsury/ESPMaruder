idf_component_register(
    SRCS
        "src/mac_utils.c"
        "src/oui_lookup.c"
        "src/time_utils.c"
        "src/led_status.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_timer
        esp_system
        esp_driver_gpio
        freertos
        log
)
