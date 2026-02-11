idf_component_register(
    SRCS
        "src/wifi_engine.c"
        "src/wifi_scanner.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_event
        esp_netif
        esp_wifi
        esp_hw_support
        log
)
