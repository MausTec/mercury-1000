#include "config.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "esp_spiffs.h"
#include <string.h>
#include "esp_log.h"

#include "cJSON.h"

static void get_str(cJSON* root, const char* key, char* buffer, size_t buflen);
static void get_bool(cJSON* root, const char* key, bool& out);
static void get_int(cJSON* root, const char* key, int& out);

static const char* TAG = "config";

config_t Config;

void config_to_json(cJSON* root, config_t* cfg) {
    cJSON_AddStringToObject(root, "wifi_ssid", cfg->wifi_ssid);
    cJSON_AddStringToObject(root, "wifi_key", cfg->wifi_key);
    cJSON_AddBoolToObject(root, "wifi_on", cfg->wifi_on);

    cJSON_AddStringToObject(root, "bt_display_name", cfg->bt_display_name);
    cJSON_AddBoolToObject(root, "bt_on", cfg->bt_on);
    cJSON_AddBoolToObject(root, "force_bt_coex", cfg->force_bt_coex);
}

void json_to_config(cJSON* root, config_t* cfg) {
    get_str(root, "wifi_ssid", cfg->wifi_ssid, WIFI_SSID_MAX_LEN);
    get_str(root, "wifi_key", cfg->wifi_key, WIFI_KEY_MAX_LEN);
    get_bool(root, "wifi_on", cfg->wifi_on);

    get_str(root, "bt_display_name", cfg->bt_display_name, 64);
    get_bool(root, "bt_on", cfg->bt_on);
    get_bool(root, "force_bt_coex", cfg->force_bt_coex);
}

//
// End Serialization Routines
//

void config_serialize(config_t* cfg, char* buf, size_t buflen) {
    cJSON* root = cJSON_CreateObject();
    config_to_json(root, cfg);
    cJSON_PrintPreallocated(root, buf, buflen, true);
    cJSON_Delete(root);
}

void config_deserialize(config_t* cfg, const char* buf) {
    cJSON* root = cJSON_Parse(buf);
    json_to_config(root, cfg);
    cJSON_Delete(root);
}

// Here There Be Dragons

void get_str(cJSON* root, const char* key, char* buffer, size_t buflen) {
    cJSON* item = cJSON_GetObjectItem(root, key);

    if (item != nullptr) {
        strncpy(buffer, item->valuestring, buflen);
    }
}

void get_bool(cJSON* root, const char* key, bool& out) {
    int value = 0;
    get_int(root, key, value);
    out = (bool) value;
}

void get_int(cJSON* root, const char* key, int& out) {
    cJSON* item = cJSON_GetObjectItem(root, key);

    if (item != nullptr) {
        out = item->valueint;
    }
}

// And here be the things declared in the header

esp_err_t config_init(void) {
    esp_vfs_spiffs_conf_t config = {
        .base_path = SPIFFS_BASE_PATH,
        .partition_label = SPIFFS_PARTITION_LABEL,
        .max_files = 5, // open at a time, not total
        .format_if_mount_failed = true
    };

    esp_err_t err = esp_vfs_spiffs_register(&config);

    if (err != ESP_OK) {
        if (err == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (err == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(err));
        }

        return err;
    } else {
        size_t total = 0, used = 0;
        err = esp_spiffs_info(config.partition_label, &total, &used);

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        }
    }

    return err;
}

esp_err_t config_load_from_nvfs(const char* filename, config_t* cfg) {
    char path[MAX_FILE_PATH_LEN + 1] = "";
    struct stat st;

    snprintf(path, MAX_FILE_PATH_LEN, SPIFFS_BASE_PATH "/%s", filename);

    if (stat(path, &st) == 0) {
        FILE* f = fopen(path, "r");
        long fsize;
        char* buffer;
        size_t result;

        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for reading: %s", path);
            return ESP_FAIL;
        }

        fseek(f, 0, SEEK_END);
        fsize = ftell(f);
        rewind(f);

        ESP_LOGD(TAG, "Allocating %d bytes for file: %s", fsize, path);
        buffer = (char*) malloc(fsize + 1);

        if (!buffer) {
            ESP_LOGE(TAG, "Failed to allocate memory for file: %s", path);
            return ESP_FAIL;
        }

        result = fread(buffer, 1, fsize, f);
        buffer[fsize] = '\0';

        if (result != fsize) {
            ESP_LOGE(TAG, "Failed to read file: %s", path);
            return ESP_FAIL;
        }
        
        ESP_LOGI(TAG, "Loaded %d bytes from %s\n%s", fsize, path, buffer);

        config_deserialize(cfg, buffer);

        fclose(f);
        free(buffer);

        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "File does not exist: %s", path);
        return ESP_FAIL;
    }
}

esp_err_t config_save_to_nvfs(const char* filename, config_t* cfg) {
    cJSON* root = cJSON_CreateObject();
    char path[MAX_FILE_PATH_LEN + 1] = "";
    struct stat st;

    config_to_json(root, cfg);
    snprintf(path, MAX_FILE_PATH_LEN, SPIFFS_BASE_PATH "/%s", filename);

    // Delete old config if it exists:
    if (stat(path, &st) == 0) {
        unlink(path);
    }

    FILE* f = fopen(path, "w");

    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
        return ESP_FAIL;
    }

    char *json = cJSON_Print(root);
    fputs(json, f);
    long fsize = ftell(f);
    fclose(f);

    ESP_LOGI(TAG, "Wrote %d bytes:\n%s", fsize, json);

    cJSON_Delete(root);
    cJSON_free(json);

    return ESP_OK;
}