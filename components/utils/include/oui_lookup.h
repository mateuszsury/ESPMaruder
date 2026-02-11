#ifndef OUI_LOOKUP_H
#define OUI_LOOKUP_H

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t oui_lookup_init(const char *db_path);
void oui_lookup_deinit(void);
const char *oui_lookup(const uint8_t mac[6]);

#ifdef __cplusplus
}
#endif

#endif  // OUI_LOOKUP_H
