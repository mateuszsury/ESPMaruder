#ifndef CLI_ENGINE_H
#define CLI_ENGINE_H

#include "esp_err.h"

esp_err_t cli_engine_init(void);
esp_err_t cli_engine_dispatch_line(const char *line);

#endif  // CLI_ENGINE_H
