#include "websocket_manager.h"
#include "esp_websocket_client.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "cJSON.h"
#include "json_api.h"

static const char* TAG = "websocket_manager";

static wm_remote_conn_status_t remote_status = WM_REMOTE_DISCONNECTED;
static char remote_device_key[41] = "";

wm_remote_conn_status_t websocket_manager_wait_for_status(int status, unsigned long wait_ms) {
    int64_t end_us = esp_timer_get_time() + (wait_ms * 1000);

    while (esp_timer_get_time() < end_us) {
        if (remote_status & status) break;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    return remote_status;
}

wm_remote_conn_status_t websocket_manager_get_status(void) {
    return remote_status;
}

void websocket_manager_register_device_key(const char *key) {
    strlcpy(remote_device_key, key, 40);
    remote_status = WM_REMOTE_CONNECTED;
}

const char *websocket_manager_get_remote_device_key(void) {
    return remote_device_key;
}

static void websocket_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_websocket_event_data_t* data = (esp_websocket_event_data_t*) event_data;
    esp_websocket_client_handle_t *client = (esp_websocket_client_handle_t*) handler_args;

    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED: {
        ESP_LOGI(TAG, "Websocket Remote connected, requesting key.");
        cJSON *response = cJSON_CreateObject();
        cJSON_AddNullToObject(response, "getDeviceKey");
        char *res_text = cJSON_Print(response);
        esp_websocket_client_send_text(client, res_text, strlen(res_text), 1000 / portTICK_RATE_MS);
        cJSON_Delete(response);
        cJSON_free(res_text);
        remote_status = WM_REMOTE_WAITING_FOR_KEY;
        break;
    }

    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Websocket Remote disconnected.");
        remote_status = WM_REMOTE_DISCONNECTED;
        break;

    case WEBSOCKET_EVENT_DATA: {
        ESP_LOGI(TAG, "Received data, opcode=%d", data->op_code);

        if (data->op_code == 0x0A) {
            ESP_LOGD(TAG, "Websocket Sends it PONGgards.");
        } else if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGI(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {
            ESP_LOGI(TAG, "Received=%.*s", data->data_len, (char*)data->data_ptr);
            char *res_text = json_api_process(data->data_ptr, data->data_len);
            esp_websocket_client_send_text(client, res_text, strlen(res_text), 1000 / portMAX_DELAY);
            json_api_free_str(res_text);
        }

        ESP_LOGD(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
        break;
    }

    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "Websocket Error Event");
        break;

    default:
        ESP_LOGW(TAG, "Unknown websocket event!");
    }
}

void websocket_manager_connect_to_remote(const char* hostname, int port) {
    char uri[120] = "";
    char protocol[4] = "ws\0";

    if (port == 443) {
        protocol[2] = 's';
    }

    snprintf(uri, 128, "%s://%s:%d/device", protocol, hostname, port);

    ESP_LOGI(TAG, "Connecting to remote: %s", uri);
    remote_status = WM_REMOTE_CONNECTING;

    const esp_websocket_client_config_t ws_cfg = {
        .uri = uri,
        .headers = NULL,
    };

    esp_websocket_client_handle_t client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*) client);
    esp_websocket_client_start(client);
}