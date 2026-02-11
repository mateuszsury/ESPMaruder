#include "oui_lookup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"

typedef struct {
    uint8_t oui[3];
    char vendor[64];
} oui_entry_t;

typedef struct {
    uint8_t oui[3];
    const char *vendor;
} oui_builtin_entry_t;

static const char *TAG = "oui_lookup";
static oui_entry_t *s_loaded_entries = NULL;
static size_t s_loaded_count = 0;

static const oui_builtin_entry_t s_builtin_entries[] = {
    {{0x24, 0x0A, 0xC4}, "Espressif Inc."},
    {{0x7C, 0xDF, 0xA1}, "Espressif Inc."},
    {{0xD8, 0xA0, 0x1D}, "Espressif Inc."},
    {{0x28, 0xCF, 0xDA}, "Apple Inc."},
    {{0x3C, 0x5A, 0xB4}, "Google LLC"},
    {{0x00, 0x1A, 0x11}, "Google LLC"},
    {{0x00, 0x50, 0x56}, "VMware, Inc."},
};

static bool parse_oui_hex(const char *text, uint8_t out_oui[3]) {
    if (text == NULL || out_oui == NULL) {
        return false;
    }

    unsigned int b0 = 0;
    unsigned int b1 = 0;
    unsigned int b2 = 0;
    if (sscanf(text, "%2x%2x%2x", &b0, &b1, &b2) != 3) {
        return false;
    }

    out_oui[0] = (uint8_t)b0;
    out_oui[1] = (uint8_t)b1;
    out_oui[2] = (uint8_t)b2;
    return true;
}

static const char *lookup_loaded(const uint8_t mac[6]) {
    for (size_t i = 0; i < s_loaded_count; i++) {
        if (memcmp(s_loaded_entries[i].oui, mac, 3) == 0) {
            return s_loaded_entries[i].vendor;
        }
    }
    return NULL;
}

static const char *lookup_builtin(const uint8_t mac[6]) {
    for (size_t i = 0; i < sizeof(s_builtin_entries) / sizeof(s_builtin_entries[0]); i++) {
        if (memcmp(s_builtin_entries[i].oui, mac, 3) == 0) {
            return s_builtin_entries[i].vendor;
        }
    }
    return NULL;
}

esp_err_t oui_lookup_init(const char *db_path) {
    oui_lookup_deinit();

    if (db_path == NULL || db_path[0] == '\0') {
        return ESP_OK;
    }

    FILE *fp = fopen(db_path, "rb");
    if (fp == NULL) {
        ESP_LOGW(TAG, "OUI DB not found: %s", db_path);
        return ESP_ERR_NOT_FOUND;
    }

    size_t capacity = 128;
    s_loaded_entries = calloc(capacity, sizeof(oui_entry_t));
    if (s_loaded_entries == NULL) {
        fclose(fp);
        return ESP_ERR_NO_MEM;
    }

    char line[192];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        char *comma = strchr(line, ',');
        if (comma == NULL) {
            continue;
        }
        *comma = '\0';

        uint8_t oui[3] = {0};
        if (!parse_oui_hex(line, oui)) {
            continue;
        }

        char *vendor = comma + 1;
        while (*vendor == ' ' || *vendor == '\t') {
            vendor++;
        }

        size_t vendor_len = strcspn(vendor, "\r\n");
        vendor[vendor_len] = '\0';
        if (vendor_len == 0) {
            continue;
        }

        if (s_loaded_count == capacity) {
            const size_t new_capacity = capacity * 2;
            oui_entry_t *new_entries = realloc(s_loaded_entries, new_capacity * sizeof(oui_entry_t));
            if (new_entries == NULL) {
                break;
            }
            s_loaded_entries = new_entries;
            capacity = new_capacity;
        }

        memcpy(s_loaded_entries[s_loaded_count].oui, oui, 3);
        snprintf(s_loaded_entries[s_loaded_count].vendor, sizeof(s_loaded_entries[s_loaded_count].vendor), "%s", vendor);
        s_loaded_count++;
    }

    fclose(fp);
    ESP_LOGI(TAG, "OUI DB loaded: entries=%u", (unsigned)s_loaded_count);
    return ESP_OK;
}

void oui_lookup_deinit(void) {
    free(s_loaded_entries);
    s_loaded_entries = NULL;
    s_loaded_count = 0;
}

const char *oui_lookup(const uint8_t mac[6]) {
    if (mac == NULL) {
        return "Unknown";
    }

    const char *vendor = lookup_loaded(mac);
    if (vendor != NULL) {
        return vendor;
    }

    vendor = lookup_builtin(mac);
    if (vendor != NULL) {
        return vendor;
    }

    return "Unknown";
}
