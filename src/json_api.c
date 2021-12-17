#include "json_api.h"
#include "websocket_manager.h"

void json_api_process_obj(cJSON* root, cJSON* response) {
    cJSON* deviceKey = cJSON_GetObjectItem(root, "deviceKey");
    if (deviceKey) {
        const char* key = cJSON_GetStringValue(deviceKey);
        websocket_manager_register_device_key(key);
    }

    cJSON* info = cJSON_GetObjectItem(root, "info");
    if (info) {
        cJSON* data = cJSON_AddObjectToObject(response, "info");
        cJSON_AddStringToObject(data, "device", "Mercury 1000");
    }
}

const char* json_api_process_str(const char* str) {
    cJSON* root = cJSON_Parse(str);
    cJSON* response = cJSON_CreateObject();
    json_api_process_obj(root, response);

    char* res_text = cJSON_Print(response);

    cJSON_Delete(response);
    cJSON_Delete(root);

    return res_text;
}

const char* json_api_process(const char* str, size_t len) {
    cJSON* root = cJSON_ParseWithLength(str, len);
    cJSON* response = cJSON_CreateObject();
    json_api_process_obj(root, response);

    char* res_text = cJSON_Print(response);

    cJSON_Delete(response);
    cJSON_Delete(root);

    return res_text;
}

void json_api_free_str(char* str) {
    cJSON_free(str);
}