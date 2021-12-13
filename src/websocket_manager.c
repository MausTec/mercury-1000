#include "websocket_manager.h"
#include "esp_websocket_client.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "cJSON.h"

static const char* TAG = "websocket_manager";

static TimerHandle_t remote_shutdown_timer;
static wm_remote_conn_status_t remote_status = WM_REMOTE_DISCONNECTED;
static char remote_device_key[41] = "";

#define REMOTE_TIMEOUT_SECONDS 30

// TODO: Refactor this into a common json_api.h file.
static void json_api_process_obj(cJSON *root, cJSON *response) {
    cJSON *deviceKey = cJSON_GetObjectItem(root, "deviceKey");
    if (deviceKey) {
        strlcpy(remote_device_key, cJSON_GetStringValue(deviceKey), 40);
        ESP_LOGI(TAG, "Got Device Key: %s", remote_device_key);
        remote_status = WM_REMOTE_CONNECTED;
    }

    cJSON *info = cJSON_GetObjectItem(root, "info");
    if (info) {
        cJSON *data = cJSON_AddObjectToObject(response, "info");
        cJSON_AddStringToObject(data, "device", "Mercury 1000");
    }
}

static void remote_shutdown_signaler(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "No data received from Remote in %d seconds, shutting down.", REMOTE_TIMEOUT_SECONDS);
}

static void websocket_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_websocket_event_data_t* data = (esp_websocket_event_data_t*) event_data;
    esp_websocket_client_handle_t *client = (esp_websocket_client_handle_t*) handler_args;

    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Websocket Remote connected.");
        remote_status = WM_REMOTE_WAITING_FOR_KEY;
        break;

    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Websocket Remote disconnected.");
        remote_status = WM_REMOTE_DISCONNECTED;
        break;

    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "Received data, opcode=%d", data->op_code);

        if (data->op_code == 0x0A) {
            ESP_LOGD(TAG, "Websocket Sends it PONGgards.");
        } else if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGI(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {
            ESP_LOGI(TAG, "Received=%.*s", data->data_len, (char*)data->data_ptr);

            cJSON *root = cJSON_ParseWithLength(data->data_ptr, data->data_len);
            cJSON *response = cJSON_CreateObject();
            json_api_process_obj(root, response);

            char *res_text = cJSON_Print(response);
            esp_websocket_client_send_text(client, res_text, strlen(res_text), 1000 / portMAX_DELAY);

            cJSON_Delete(response);
            cJSON_free(res_text);
            cJSON_Delete(root);
        }

        ESP_LOGD(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        xTimerReset(remote_shutdown_timer, portMAX_DELAY);
        break;

    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "Websocket Error Event");
        break;

    default:
        ESP_LOGW(TAG, "Unknown websocket event!");
    }
}

void websocket_manager_connect_to_remote(const char* hostname, int port) {
    char uri[120] = "";
    snprintf(uri, 128, "ws://%s:%d/device", hostname, port);

    ESP_LOGI(TAG, "Connecting to remote: %s", uri);
    remote_status = WM_REMOTE_CONNECTING;

    const esp_websocket_client_config_t ws_cfg = {
        .uri = uri,
    };

    remote_shutdown_timer = xTimerCreate(
        "Websocket Remote Shutdown",
        REMOTE_TIMEOUT_SECONDS * 1000 / portTICK_PERIOD_MS,
        pdFALSE,
        NULL,
        remote_shutdown_signaler
    );

    esp_websocket_client_handle_t client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*) client);

    esp_websocket_client_start(client);
    xTimerStart(remote_shutdown_timer, portMAX_DELAY);
}