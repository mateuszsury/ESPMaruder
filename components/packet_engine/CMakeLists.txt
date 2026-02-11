idf_component_register(
    SRCS
        "src/packet_buffer.c"
        "src/pcap_writer.c"
        "src/packet_filter.c"
        "src/packet_stats.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_hw_support
        log
)
