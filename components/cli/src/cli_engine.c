#include "cli_engine.h"

#include <ctype.h>
#include <dirent.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "driver/temperature_sensor.h"
#include "driver/uart.h"
#include "esp_app_desc.h"
#include "esp_check.h"
#include "esp_chip_info.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "sdkconfig.h"

#include "cli_colors.h"
#include "nvs_mgr.h"
#include "sd_card.h"

#define CLI_MAX_CMDLINE_LEN 256
#define CLI_MAX_CMDLINE_ARGS 16
#define CLI_MAX_NVS_VALUE_LEN 192
#define CLI_MAX_SCRIPT_DEPTH 4
#define CLI_HISTORY_DEPTH 16
#define CLI_TASK_STACK_SIZE 6144
#define CLI_TASK_PRIORITY 5

#ifndef SPECTRA_FW_BUILD_PROFILE_CSTR
#define SPECTRA_FW_BUILD_PROFILE_CSTR "debug"
#endif

typedef int (*cli_handler_t)(int argc, char **argv);

typedef struct {
    const char *name;
    const char *help;
    const char *usage;
    cli_handler_t handler;
} cli_command_t;

static const char *TAG = "cli_engine";
static TaskHandle_t s_cli_task = NULL;
static int s_script_depth = 0;
static char s_history[CLI_HISTORY_DEPTH][CLI_MAX_CMDLINE_LEN];
static size_t s_history_count = 0;
static size_t s_history_next = 0;

static int cmd_help(int argc, char **argv);
static int cmd_version(int argc, char **argv);
static int cmd_sysinfo(int argc, char **argv);
static int cmd_reboot(int argc, char **argv);
static int cmd_sd(int argc, char **argv);
static int cmd_set(int argc, char **argv);
static int cmd_get(int argc, char **argv);
static int cmd_script(int argc, char **argv);

static const cli_command_t s_commands[] = {
    {.name = "help", .help = "Print command list", .usage = "help", .handler = &cmd_help},
    {.name = "version", .help = "Print firmware and build version details", .usage = "version", .handler = &cmd_version},
    {.name = "sysinfo", .help = "Print runtime system diagnostics", .usage = "sysinfo", .handler = &cmd_sysinfo},
    {.name = "reboot", .help = "Restart the device", .usage = "reboot", .handler = &cmd_reboot},
    {.name = "sd", .help = "SD operations: ls|cat|rm", .usage = "sd <ls|cat|rm> <path>", .handler = &cmd_sd},
    {.name = "set", .help = "Store key/value in NVS namespace 'spectra'", .usage = "set <key> <value>", .handler = &cmd_set},
    {.name = "get", .help = "Read key from NVS namespace 'spectra'", .usage = "get <key>", .handler = &cmd_get},
    {.name = "script", .help = "Run CLI script from SD file", .usage = "script <file>", .handler = &cmd_script},
};

static const size_t s_command_count = sizeof(s_commands) / sizeof(s_commands[0]);

static void cli_prompt(void) {
    printf(CLI_COLOR_GREEN "spectra> " CLI_COLOR_RESET);
    fflush(stdout);
}

static void cli_history_push(const char *line) {
    if (line == NULL || line[0] == '\0') {
        return;
    }

    strncpy(s_history[s_history_next], line, CLI_MAX_CMDLINE_LEN - 1);
    s_history[s_history_next][CLI_MAX_CMDLINE_LEN - 1] = '\0';

    s_history_next = (s_history_next + 1) % CLI_HISTORY_DEPTH;
    if (s_history_count < CLI_HISTORY_DEPTH) {
        s_history_count++;
    }
}

static char *trim_line(char *line) {
    if (line == NULL) {
        return NULL;
    }

    while (*line != '\0' && isspace((unsigned char)*line)) {
        line++;
    }

    char *end = line + strlen(line);
    while (end > line && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return line;
}

static int split_args(char *line, char **argv, int max_args) {
    int argc = 0;
    char *p = line;
    while (*p != '\0' && argc < max_args) {
        while (*p != '\0' && isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        argv[argc++] = p;
        while (*p != '\0' && !isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        *p++ = '\0';
    }
    return argc;
}

static const cli_command_t *find_command(const char *name) {
    if (name == NULL || name[0] == '\0') {
        return NULL;
    }

    for (size_t i = 0; i < s_command_count; i++) {
        if (strcmp(s_commands[i].name, name) == 0) {
            return &s_commands[i];
        }
    }
    return NULL;
}

static bool nvs_key_is_valid(const char *key) {
    if (key == NULL) {
        return false;
    }

    const size_t key_len = strlen(key);
    if (key_len == 0 || key_len > 15) {
        return false;
    }

    for (size_t i = 0; i < key_len; i++) {
        const char c = key[i];
        if (!isalnum((unsigned char)c) && c != '_' && c != '-') {
            return false;
        }
    }
    return true;
}

static esp_err_t sd_make_abs_path(const char *path_in, char *path_out, size_t path_out_len) {
    if (path_out == NULL || path_out_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    const char *rel = (path_in == NULL || path_in[0] == '\0') ? "/" : path_in;
    int written = 0;
    if (rel[0] == '/') {
        written = snprintf(path_out, path_out_len, "%s%s", SPECTRA_SD_BASE_PATH, rel);
    } else {
        written = snprintf(path_out, path_out_len, "%s/%s", SPECTRA_SD_BASE_PATH, rel);
    }

    if (written < 0 || (size_t)written >= path_out_len) {
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

static bool read_chip_temperature(float *out_temp_c) {
    if (out_temp_c == NULL) {
        return false;
    }

    temperature_sensor_handle_t sensor = NULL;
    temperature_sensor_config_t cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
    if (temperature_sensor_install(&cfg, &sensor) != ESP_OK) {
        return false;
    }

    bool ok = false;
    if (temperature_sensor_enable(sensor) == ESP_OK) {
        if (temperature_sensor_get_celsius(sensor, out_temp_c) == ESP_OK) {
            ok = true;
        }
        (void)temperature_sensor_disable(sensor);
    }

    (void)temperature_sensor_uninstall(sensor);
    return ok;
}

static int cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("Available commands:\n");
    for (size_t i = 0; i < s_command_count; i++) {
        printf("  %-8s - %s\n", s_commands[i].name, s_commands[i].help);
        printf("    usage: %s\n", s_commands[i].usage);
    }
    return 0;
}

static int cmd_version(int argc, char **argv) {
    (void)argc;
    (void)argv;

    const esp_app_desc_t *app = esp_app_get_description();
    printf("project: %s\n", app->project_name);
    printf("version: %s\n", app->version);
    printf("idf: %s\n", app->idf_ver);
    printf("build_profile: %s\n", SPECTRA_FW_BUILD_PROFILE_CSTR);
    return 0;
}

static int cmd_sysinfo(int argc, char **argv) {
    (void)argc;
    (void)argv;

    const uint64_t uptime_sec = (uint64_t)esp_timer_get_time() / 1000000ULL;
    const uint32_t free_heap = esp_get_free_heap_size();
    const uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    const size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    const size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("uptime_sec: %" PRIu64 "\n", uptime_sec);
    printf("chip_cores: %u\n", chip_info.cores);
    printf("free_heap: %" PRIu32 "\n", free_heap);
    printf("min_free_heap: %" PRIu32 "\n", min_free_heap);
    printf("free_internal: %zu\n", free_internal);
    printf("free_psram: %zu\n", free_psram);

    float temp_c = 0.0f;
    if (read_chip_temperature(&temp_c)) {
        printf("temperature_c: %.2f\n", (double)temp_c);
    } else {
        printf("temperature_c: unavailable\n");
    }

    return 0;
}

static int cmd_reboot(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("rebooting...\n");
    fflush(stdout);
    esp_restart();
    return 0;
}

static int cmd_set(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: set <key> <value>\n");
        return 1;
    }

    const char *key = argv[1];
    const char *value = argv[2];

    if (!nvs_key_is_valid(key)) {
        printf("invalid key: use [A-Za-z0-9_-], max 15 chars\n");
        return 1;
    }

    esp_err_t err = nvs_mgr_store_string(key, value);
    if (err != ESP_OK) {
        printf("set failed: %s\n", esp_err_to_name(err));
        return 1;
    }

    printf("ok\n");
    return 0;
}

static int cmd_get(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: get <key>\n");
        return 1;
    }

    const char *key = argv[1];
    if (!nvs_key_is_valid(key)) {
        printf("invalid key: use [A-Za-z0-9_-], max 15 chars\n");
        return 1;
    }

    char value[CLI_MAX_NVS_VALUE_LEN];
    size_t value_len = sizeof(value);
    esp_err_t err = nvs_mgr_load_string(key, value, &value_len);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        printf("not found\n");
        return 1;
    }
    if (err != ESP_OK) {
        printf("get failed: %s\n", esp_err_to_name(err));
        return 1;
    }

    printf("%s=%s\n", key, value);
    return 0;
}

static int cmd_sd_ls_inner(const char *path) {
    if (!sd_card_is_mounted()) {
        printf("sd not mounted\n");
        return 1;
    }

    char abs_path[256];
    esp_err_t err = sd_make_abs_path(path, abs_path, sizeof(abs_path));
    if (err != ESP_OK) {
        printf("invalid path\n");
        return 1;
    }

    DIR *dir = opendir(abs_path);
    if (dir == NULL) {
        printf("cannot open dir: %s\n", abs_path);
        return 1;
    }

    size_t count = 0;
    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char child_path[320];
        int written = snprintf(child_path, sizeof(child_path), "%s/%s", abs_path, entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(child_path)) {
            continue;
        }

        struct stat st = {0};
        const bool has_stat = stat(child_path, &st) == 0;
        const char kind = (has_stat && S_ISDIR(st.st_mode)) ? 'd' : '-';
        const long long size = has_stat ? (long long)st.st_size : -1LL;

        printf("%c %10lld %s\n", kind, size, entry->d_name);
        count++;
    }

    closedir(dir);
    printf("entries: %zu\n", count);
    return 0;
}

static int cmd_sd_cat_inner(const char *path) {
    if (!sd_card_is_mounted()) {
        printf("sd not mounted\n");
        return 1;
    }

    char abs_path[256];
    esp_err_t err = sd_make_abs_path(path, abs_path, sizeof(abs_path));
    if (err != ESP_OK) {
        printf("invalid path\n");
        return 1;
    }

    FILE *fp = fopen(abs_path, "rb");
    if (fp == NULL) {
        printf("cannot open file: %s\n", abs_path);
        return 1;
    }

    char buf[128];
    size_t read_len = 0;
    while ((read_len = fread(buf, 1, sizeof(buf), fp)) > 0) {
        fwrite(buf, 1, read_len, stdout);
    }
    fclose(fp);
    printf("\n");
    return 0;
}

static int cmd_sd_rm_inner(const char *path) {
    if (!sd_card_is_mounted()) {
        printf("sd not mounted\n");
        return 1;
    }

    char abs_path[256];
    esp_err_t err = sd_make_abs_path(path, abs_path, sizeof(abs_path));
    if (err != ESP_OK) {
        printf("invalid path\n");
        return 1;
    }

    if (unlink(abs_path) != 0) {
        printf("remove failed: %s\n", abs_path);
        return 1;
    }

    printf("removed: %s\n", abs_path);
    return 0;
}

static int cmd_sd(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: sd <ls|cat|rm> <path>\n");
        return 1;
    }

    const char *sub = argv[1];
    if (strcmp(sub, "ls") == 0) {
        const char *path = (argc >= 3) ? argv[2] : "/";
        return cmd_sd_ls_inner(path);
    }
    if (strcmp(sub, "cat") == 0) {
        if (argc < 3) {
            printf("usage: sd cat <file>\n");
            return 1;
        }
        return cmd_sd_cat_inner(argv[2]);
    }
    if (strcmp(sub, "rm") == 0) {
        if (argc < 3) {
            printf("usage: sd rm <file>\n");
            return 1;
        }
        return cmd_sd_rm_inner(argv[2]);
    }

    printf("unknown sd subcommand: %s\n", sub);
    return 1;
}

static int cmd_script(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: script <file>\n");
        return 1;
    }

    if (s_script_depth >= CLI_MAX_SCRIPT_DEPTH) {
        printf("script depth limit reached (%d)\n", CLI_MAX_SCRIPT_DEPTH);
        return 1;
    }
    if (!sd_card_is_mounted()) {
        printf("sd not mounted\n");
        return 1;
    }

    char abs_path[256];
    esp_err_t err = sd_make_abs_path(argv[1], abs_path, sizeof(abs_path));
    if (err != ESP_OK) {
        printf("invalid path\n");
        return 1;
    }

    FILE *fp = fopen(abs_path, "rb");
    if (fp == NULL) {
        printf("cannot open script: %s\n", abs_path);
        return 1;
    }

    s_script_depth++;
    int result = 0;
    char line[CLI_MAX_CMDLINE_LEN];
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *trimmed = trim_line(line);
        if (trimmed == NULL || trimmed[0] == '\0' || trimmed[0] == '#') {
            continue;
        }

        printf("script> %s\n", trimmed);
        err = cli_engine_dispatch_line(trimmed);
        if (err != ESP_OK) {
            printf("script aborted: %s\n", esp_err_to_name(err));
            result = 1;
            break;
        }
    }

    s_script_depth--;
    fclose(fp);
    return result;
}

esp_err_t cli_engine_dispatch_line(const char *line) {
    if (line == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char work[CLI_MAX_CMDLINE_LEN];
    strncpy(work, line, sizeof(work) - 1);
    work[sizeof(work) - 1] = '\0';

    char *trimmed = trim_line(work);
    if (trimmed == NULL || trimmed[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }

    char *argv[CLI_MAX_CMDLINE_ARGS] = {0};
    const int argc = split_args(trimmed, argv, CLI_MAX_CMDLINE_ARGS);
    if (argc <= 0) {
        return ESP_ERR_INVALID_ARG;
    }

    const cli_command_t *cmd = find_command(argv[0]);
    if (cmd == NULL) {
        printf(CLI_COLOR_RED "unknown command" CLI_COLOR_RESET ": %s\n", argv[0]);
        return ESP_ERR_NOT_FOUND;
    }

    const int rc = cmd->handler(argc, argv);
    return (rc == 0) ? ESP_OK : ESP_FAIL;
}

static void cli_task(void *arg) {
    (void)arg;

    char line[CLI_MAX_CMDLINE_LEN];
    size_t len = 0;
    memset(line, 0, sizeof(line));

    printf("\n");
    cli_prompt();

    while (true) {
        uint8_t ch = 0;
        const int rd = uart_read_bytes((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, &ch, 1, pdMS_TO_TICKS(50));
        if (rd <= 0) {
            continue;
        }

        if (ch == '\r' || ch == '\n') {
            if (len == 0) {
                printf("\n");
                cli_prompt();
                continue;
            }

            line[len] = '\0';
            printf("\n");

            char staged[CLI_MAX_CMDLINE_LEN];
            strncpy(staged, line, sizeof(staged) - 1);
            staged[sizeof(staged) - 1] = '\0';
            char *trimmed = trim_line(staged);

            if (trimmed != NULL && trimmed[0] != '\0') {
                cli_history_push(trimmed);
                esp_err_t err = cli_engine_dispatch_line(trimmed);
                if (err != ESP_OK && err != ESP_FAIL && err != ESP_ERR_NOT_FOUND) {
                    printf(CLI_COLOR_RED "dispatch error" CLI_COLOR_RESET ": %s\n", esp_err_to_name(err));
                }
            }

            len = 0;
            line[0] = '\0';
            cli_prompt();
            continue;
        }

        if (ch == 0x08 || ch == 0x7f) {
            if (len > 0) {
                len--;
                line[len] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        if (isprint(ch) && len < (CLI_MAX_CMDLINE_LEN - 1)) {
            line[len++] = (char)ch;
            putchar((int)ch);
            fflush(stdout);
        }
    }
}

esp_err_t cli_engine_init(void) {
    if (s_cli_task != NULL) {
        return ESP_OK;
    }

    const uart_port_t port = (uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM;
    const uart_config_t uart_cfg = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_RETURN_ON_ERROR(uart_param_config(port, &uart_cfg), TAG, "uart_param_config failed");
    ESP_RETURN_ON_ERROR(uart_set_pin(port, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE),
                        TAG, "uart_set_pin failed");

    if (!uart_is_driver_installed(port)) {
        ESP_RETURN_ON_ERROR(uart_driver_install(port, 1024, 0, 0, NULL, 0), TAG, "uart_driver_install failed");
    }

    BaseType_t task_ok = xTaskCreate(cli_task, "spectra_cli", CLI_TASK_STACK_SIZE, NULL, CLI_TASK_PRIORITY, &s_cli_task);
    if (task_ok != pdPASS) {
        s_cli_task = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "CLI engine ready. Authorized lab use on owned devices only.");
    return ESP_OK;
}
