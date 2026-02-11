idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES core_system storage cli gui esp_event nvs_flash esp_system
)

if(DEFINED SPECTRA_BUILD_PROFILE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE SPECTRA_FW_BUILD_PROFILE_CSTR=\"${SPECTRA_BUILD_PROFILE}\")
endif()
