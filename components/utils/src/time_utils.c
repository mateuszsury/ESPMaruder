#include "time_utils.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "esp_timer.h"

uint64_t time_get_us(void) {
    return (uint64_t)esp_timer_get_time();
}

esp_err_t time_get_iso8601(char *buf, size_t buf_len) {
    if (buf == NULL || buf_len < 21) {
        return ESP_ERR_INVALID_SIZE;
    }

    time_t now = time(NULL);
    struct tm tm_now = {0};
    if (gmtime_r(&now, &tm_now) == NULL) {
        return ESP_FAIL;
    }

    const int written = snprintf(buf, buf_len, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                                 tm_now.tm_year + 1900,
                                 tm_now.tm_mon + 1,
                                 tm_now.tm_mday,
                                 tm_now.tm_hour,
                                 tm_now.tm_min,
                                 tm_now.tm_sec);
    if (written < 0 || (size_t)written >= buf_len) {
        return ESP_ERR_INVALID_SIZE;
    }

    return ESP_OK;
}

esp_err_t time_set_from_gps(const nmea_time_t *nmea_time) {
    if (nmea_time == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (nmea_time->year < 2000 || nmea_time->year > 2099 ||
        nmea_time->month < 1 || nmea_time->month > 12 ||
        nmea_time->day < 1 || nmea_time->day > 31 ||
        nmea_time->hour < 0 || nmea_time->hour > 23 ||
        nmea_time->minute < 0 || nmea_time->minute > 59 ||
        nmea_time->second < 0 || nmea_time->second > 60) {
        return ESP_ERR_INVALID_ARG;
    }

    struct tm tm_gps = {
        .tm_year = nmea_time->year - 1900,
        .tm_mon = nmea_time->month - 1,
        .tm_mday = nmea_time->day,
        .tm_hour = nmea_time->hour,
        .tm_min = nmea_time->minute,
        .tm_sec = nmea_time->second,
    };

    time_t epoch = mktime(&tm_gps);
    if (epoch < 0) {
        return ESP_FAIL;
    }

    struct timeval tv = {
        .tv_sec = epoch,
        .tv_usec = 0,
    };

    if (settimeofday(&tv, NULL) != 0) {
        return ESP_FAIL;
    }

    return ESP_OK;
}
