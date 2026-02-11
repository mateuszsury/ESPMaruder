#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} nmea_time_t;

uint64_t time_get_us(void);
esp_err_t time_get_iso8601(char *buf, size_t buf_len);
esp_err_t time_set_from_gps(const nmea_time_t *nmea_time);

#ifdef __cplusplus
}
#endif

#endif  // TIME_UTILS_H
