#ifndef __config_hpp
#define __config_hpp

#include "esp_err.h"
#include <stddef.h>

#define SPIFFS_BASE_PATH "/spiffs"
#define SPIFFS_PARTITION_LABEL "spiffs"
#define CONFIG_DEFAULT_FILE "config.json"

#define WIFI_SSID_MAX_LEN 64
#define WIFI_KEY_MAX_LEN 64
#define MAX_FILE_PATH_LEN 64

struct config {
    char wifi_ssid[WIFI_SSID_MAX_LEN + 1] = "";
    char wifi_key[WIFI_KEY_MAX_LEN + 1] = "";
    bool wifi_on = false;

    char bt_display_name[64] = "";
    bool bt_on = false;
    bool force_bt_coex = false;
};

typedef struct config config_t;

extern config_t Config;

esp_err_t config_init(void);

esp_err_t config_load_from_nvfs(const char *filename, config_t *cfg);
esp_err_t config_save_to_nvfs(const char *filename, config_t *cfg);

void config_serialize(config_t *cfg, char *buf, size_t buflen);
void config_deserialize(config_t *cfg, const char *buf);

bool config_set(const char *key, const char *value, bool &require_reboot);
bool config_get(const char *key, char *value, size_t value_len);

#endif