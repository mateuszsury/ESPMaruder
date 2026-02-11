#ifndef MAC_UTILS_H
#define MAC_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAC_STR_LEN 18U

esp_err_t mac_to_str(const uint8_t mac[6], char *buf, size_t buf_len);
esp_err_t str_to_mac(const char *str, uint8_t mac[6]);
void mac_randomize(uint8_t mac[6]);
bool mac_is_broadcast(const uint8_t mac[6]);
bool mac_is_multicast(const uint8_t mac[6]);

#ifdef __cplusplus
}
#endif

#endif  // MAC_UTILS_H
