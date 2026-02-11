idf_component_register(
    SRCS
        "src/spectra_cli.c"
        "src/cli_engine.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_driver_uart
        esp_driver_tsens
        esp_app_format
        esp_system
        esp_timer
        esp_hw_support
        vfs
        storage
)

if(DEFINED SPECTRA_BUILD_PROFILE)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC SPECTRA_FW_BUILD_PROFILE_CSTR="${SPECTRA_BUILD_PROFILE}")
endif()
