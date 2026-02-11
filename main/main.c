#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "gui_main.h"
#include "spectra_cli.h"
#include "spectra_storage.h"
#include "spectra_system.h"
#include "version.h"

static const char *TAG = "spectra_main";

static void print_banner(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("\n");
    printf("========================================\n");
    printf("  %s firmware %s\n", SPECTRA_FW_NAME, SPECTRA_FW_VERSION);
    printf("  Build profile: %s\n", SPECTRA_FW_BUILD_PROFILE);
    printf("  IDF: %s\n", esp_get_idf_version());
    printf("  Chip cores: %u\n", chip_info.cores);
    printf("========================================\n");
    printf("LEGAL NOTICE (EN): Authorized lab testing on owned devices only.\n");
    printf("NOTA PRAWNA (PL): Wylacznie testy laboratoryjne na wlasnych urzadzeniach.\n");
    printf("========================================\n");
}

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(spectra_system_init());
    ESP_ERROR_CHECK(spectra_storage_init());
    ESP_ERROR_CHECK(gui_init());
    ESP_ERROR_CHECK(spectra_cli_init());

    print_banner();
    ESP_LOGI(TAG, "SPECTRA bootstrap complete");
}
