#include "pcap_writer.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"

typedef struct {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
} pcap_global_header_t;

typedef struct {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
} pcap_record_header_t;

static const char *TAG = "pcap_writer";

static bool link_type_supported(pcap_link_type_t link_type) {
    return link_type == DLT_IEEE802_11 ||
           link_type == DLT_IEEE802_11_RADIO ||
           link_type == DLT_IEEE802_15_4 ||
           link_type == DLT_BLUETOOTH_LE_LL;
}

static esp_err_t build_split_path(const pcap_writer_t *writer, uint32_t split_index, char *out_path, size_t out_path_len) {
    if (writer == NULL || out_path == NULL || out_path_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (split_index == 0) {
        const int written = snprintf(out_path, out_path_len, "%s", writer->base_path);
        if (written < 0 || (size_t)written >= out_path_len) {
            return ESP_ERR_INVALID_SIZE;
        }
        return ESP_OK;
    }

    const char *last_slash = strrchr(writer->base_path, '/');
    const char *last_backslash = strrchr(writer->base_path, '\\');
    const char *path_sep = (last_slash > last_backslash) ? last_slash : last_backslash;
    const char *file_start = (path_sep == NULL) ? writer->base_path : path_sep + 1;

    const char *dot = strrchr(file_start, '.');
    const char *ext = (dot == NULL) ? "" : dot;

    char prefix[192] = {0};
    if (dot == NULL) {
        const int prefix_len = snprintf(prefix, sizeof(prefix), "%s", writer->base_path);
        if (prefix_len < 0 || (size_t)prefix_len >= sizeof(prefix)) {
            return ESP_ERR_INVALID_SIZE;
        }
    } else {
        const size_t prefix_len = (size_t)(dot - writer->base_path);
        if (prefix_len >= sizeof(prefix)) {
            return ESP_ERR_INVALID_SIZE;
        }
        memcpy(prefix, writer->base_path, prefix_len);
        prefix[prefix_len] = '\0';
    }

    const int written = snprintf(out_path, out_path_len, "%s_%04u%s", prefix, (unsigned)split_index, ext);
    if (written < 0 || (size_t)written >= out_path_len) {
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

static esp_err_t write_global_header(FILE *fp, uint32_t snaplen, pcap_link_type_t link_type) {
    pcap_global_header_t gh = {
        .magic_number = 0xa1b2c3d4U,
        .version_major = 2,
        .version_minor = 4,
        .thiszone = 0,
        .sigfigs = 0,
        .snaplen = snaplen,
        .network = (uint32_t)link_type,
    };

    const size_t written = fwrite(&gh, 1, sizeof(gh), fp);
    if (written != sizeof(gh)) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t open_split_file(pcap_writer_t *writer) {
    char path[sizeof(writer->current_path)];
    esp_err_t err = build_split_path(writer, writer->split_index, path, sizeof(path));
    if (err != ESP_OK) {
        return err;
    }

    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        ESP_LOGE(TAG, "fopen failed: %s", path);
        return ESP_FAIL;
    }

    err = write_global_header(fp, writer->snaplen, writer->link_type);
    if (err != ESP_OK) {
        fclose(fp);
        ESP_LOGE(TAG, "global header write failed: %s", path);
        return err;
    }

    writer->file = fp;
    writer->bytes_written_current_file = sizeof(pcap_global_header_t);
    writer->packets_written_current_file = 0;
    snprintf(writer->current_path, sizeof(writer->current_path), "%s", path);

    ESP_LOGI(TAG, "Opened PCAP file: %s", writer->current_path);
    return ESP_OK;
}

esp_err_t pcap_writer_open(pcap_writer_t *writer, const char *filepath, pcap_link_type_t link_type) {
    return pcap_writer_open_ex(writer, filepath, link_type, PCAP_WRITER_DEFAULT_SNAPLEN, PCAP_WRITER_DEFAULT_SPLIT_BYTES);
}

esp_err_t pcap_writer_open_ex(pcap_writer_t *writer,
                              const char *filepath,
                              pcap_link_type_t link_type,
                              uint32_t snaplen,
                              uint32_t split_threshold_bytes) {
    if (writer == NULL || filepath == NULL || filepath[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }
    if (!link_type_supported(link_type)) {
        return ESP_ERR_NOT_SUPPORTED;
    }
    if (snaplen == 0) {
        return ESP_ERR_INVALID_SIZE;
    }
    if (writer->opened) {
        return ESP_ERR_INVALID_STATE;
    }

    memset(writer, 0, sizeof(*writer));
    const int base_written = snprintf(writer->base_path, sizeof(writer->base_path), "%s", filepath);
    if (base_written < 0 || (size_t)base_written >= sizeof(writer->base_path)) {
        return ESP_ERR_INVALID_SIZE;
    }

    writer->snaplen = snaplen;
    writer->split_threshold_bytes = split_threshold_bytes;
    writer->link_type = link_type;
    writer->split_index = 0;

    esp_err_t err = open_split_file(writer);
    if (err != ESP_OK) {
        memset(writer, 0, sizeof(*writer));
        return err;
    }

    writer->opened = true;
    return ESP_OK;
}

esp_err_t pcap_writer_write_packet(pcap_writer_t *writer,
                                   const uint8_t *data,
                                   size_t length,
                                   uint64_t timestamp_us) {
    if (writer == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!writer->opened || writer->file == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (length == 0 || length > UINT32_MAX) {
        return ESP_ERR_INVALID_SIZE;
    }

    const uint32_t incl_len = (length > writer->snaplen) ? writer->snaplen : (uint32_t)length;
    const uint64_t required = sizeof(pcap_record_header_t) + (uint64_t)incl_len;

    if (writer->split_threshold_bytes > 0 &&
        writer->bytes_written_current_file > 0 &&
        writer->bytes_written_current_file + required > writer->split_threshold_bytes &&
        writer->packets_written_current_file > 0) {
        fclose(writer->file);
        writer->file = NULL;
        writer->split_index++;

        esp_err_t open_err = open_split_file(writer);
        if (open_err != ESP_OK) {
            return open_err;
        }
    }

    pcap_record_header_t rh = {
        .ts_sec = (uint32_t)(timestamp_us / 1000000ULL),
        .ts_usec = (uint32_t)(timestamp_us % 1000000ULL),
        .incl_len = incl_len,
        .orig_len = (uint32_t)length,
    };

    size_t written = fwrite(&rh, 1, sizeof(rh), writer->file);
    if (written != sizeof(rh)) {
        return ESP_FAIL;
    }

    written = fwrite(data, 1, incl_len, writer->file);
    if (written != incl_len) {
        return ESP_FAIL;
    }

    writer->bytes_written_current_file += required;
    writer->packets_written_current_file++;
    writer->bytes_written_total += required;
    writer->packets_written_total++;

    return ESP_OK;
}

esp_err_t pcap_writer_close(pcap_writer_t *writer) {
    if (writer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!writer->opened) {
        return ESP_OK;
    }

    if (writer->file != NULL) {
        fflush(writer->file);
        fclose(writer->file);
    }

    memset(writer, 0, sizeof(*writer));
    return ESP_OK;
}
