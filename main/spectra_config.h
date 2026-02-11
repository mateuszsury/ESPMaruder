#ifndef SPECTRA_CONFIG_H
#define SPECTRA_CONFIG_H

#include "esp_heap_caps.h"

// Feature toggles (phase 1 defaults)
#define SPECTRA_SD_ENABLED 1
#define SPECTRA_GPS_ENABLED 0
#define SPECTRA_WEB_UI_ENABLED 0

// Buffer sizing
#define SPECTRA_PACKET_BUFFER_SIZE 4096

// Placeholder pins to be finalized in hardware phase
#define SPECTRA_SD_CS_PIN 10
#define SPECTRA_GPS_TX_PIN 17
#define SPECTRA_GPS_RX_PIN 18
#define SPECTRA_LED_PIN 2

// PSRAM helpers
#define SPECTRA_MALLOC_PSRAM(size) \
    heap_caps_malloc((size), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define SPECTRA_CALLOC_PSRAM(n, size) \
    heap_caps_calloc((n), (size), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define SPECTRA_REALLOC_PSRAM(ptr, size) \
    heap_caps_realloc((ptr), (size), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define SPECTRA_MALLOC_DMA(size) \
    heap_caps_malloc((size), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)
#define SPECTRA_PSRAM_FREE() \
    heap_caps_get_free_size(MALLOC_CAP_SPIRAM)

// Memory thresholds
#define SPECTRA_PSRAM_LOW_THRESHOLD (256 * 1024)
#define SPECTRA_PSRAM_CRITICAL_THRESHOLD (64 * 1024)

#endif  // SPECTRA_CONFIG_H

