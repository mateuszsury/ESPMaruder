idf_component_register(
    SRCS
        "src/spectra_storage.c"
        "src/nvs_mgr.c"
        "src/spiffs_mgr.c"
        "src/sd_card.c"
        "src/log_rotate.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash spiffs fatfs sdmmc esp_driver_sdspi esp_driver_spi esp_driver_gpio
)
