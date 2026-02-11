#ifndef PCAP_WRITER_H
#define PCAP_WRITER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCAP_WRITER_DEFAULT_SNAPLEN 65535U
#define PCAP_WRITER_DEFAULT_SPLIT_BYTES (50U * 1024U * 1024U)

typedef enum {
    DLT_IEEE802_11 = 105,
    DLT_IEEE802_11_RADIO = 127,
    DLT_IEEE802_15_4 = 195,
    DLT_BLUETOOTH_LE_LL = 251,
} pcap_link_type_t;

typedef struct {
    FILE *file;
    char base_path[192];
    char current_path[224];
    uint32_t split_index;
    uint32_t split_threshold_bytes;
    uint32_t snaplen;
    pcap_link_type_t link_type;
    uint64_t bytes_written_total;
    uint64_t packets_written_total;
    uint64_t bytes_written_current_file;
    uint64_t packets_written_current_file;
    bool opened;
} pcap_writer_t;

esp_err_t pcap_writer_open(pcap_writer_t *writer, const char *filepath, pcap_link_type_t link_type);
esp_err_t pcap_writer_open_ex(pcap_writer_t *writer,
                              const char *filepath,
                              pcap_link_type_t link_type,
                              uint32_t snaplen,
                              uint32_t split_threshold_bytes);
esp_err_t pcap_writer_write_packet(pcap_writer_t *writer,
                                   const uint8_t *data,
                                   size_t length,
                                   uint64_t timestamp_us);
esp_err_t pcap_writer_close(pcap_writer_t *writer);

#ifdef __cplusplus
}
#endif

#endif  // PCAP_WRITER_H
