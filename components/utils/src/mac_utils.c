#include "mac_utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "esp_random.h"

static int hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    c = (char)toupper((unsigned char)c);
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    return -1;
}

esp_err_t mac_to_str(const uint8_t mac[6], char *buf, size_t buf_len) {
    if (mac == NULL || buf == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (buf_len < MAC_STR_LEN) {
        return ESP_ERR_INVALID_SIZE;
    }

    const int written = snprintf(buf, buf_len, "%02X:%02X:%02X:%02X:%02X:%02X",
                                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    if (written < 0 || (size_t)written >= buf_len) {
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

esp_err_t str_to_mac(const char *str, uint8_t mac[6]) {
    if (str == NULL || mac == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const size_t len = strlen(str);
    if (len != 17) {
        return ESP_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < 17; i++) {
        const bool sep = (i == 2 || i == 5 || i == 8 || i == 11 || i == 14);
        if (sep) {
            if (str[i] != ':') {
                return ESP_ERR_INVALID_ARG;
            }
            continue;
        }
        if (!isxdigit((unsigned char)str[i])) {
            return ESP_ERR_INVALID_ARG;
        }
    }

    for (size_t i = 0, j = 0; i < 6; i++, j += 3) {
        const int hi = hex_value(str[j]);
        const int lo = hex_value(str[j + 1]);
        if (hi < 0 || lo < 0) {
            return ESP_ERR_INVALID_ARG;
        }
        mac[i] = (uint8_t)((hi << 4) | lo);
    }

    return ESP_OK;
}

void mac_randomize(uint8_t mac[6]) {
    if (mac == NULL) {
        return;
    }

    uint32_t r0 = esp_random();
    uint32_t r1 = esp_random();
    mac[0] = (uint8_t)(r0 & 0xFFU);
    mac[1] = (uint8_t)((r0 >> 8) & 0xFFU);
    mac[2] = (uint8_t)((r0 >> 16) & 0xFFU);
    mac[3] = (uint8_t)((r0 >> 24) & 0xFFU);
    mac[4] = (uint8_t)(r1 & 0xFFU);
    mac[5] = (uint8_t)((r1 >> 8) & 0xFFU);

    mac[0] |= 0x02U;
    mac[0] &= 0xFEU;
}

bool mac_is_broadcast(const uint8_t mac[6]) {
    if (mac == NULL) {
        return false;
    }

    for (size_t i = 0; i < 6; i++) {
        if (mac[i] != 0xFFU) {
            return false;
        }
    }
    return true;
}

bool mac_is_multicast(const uint8_t mac[6]) {
    if (mac == NULL) {
        return false;
    }
    return (mac[0] & 0x01U) != 0U;
}
