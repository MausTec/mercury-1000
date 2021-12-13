#ifndef __websocket_manager_h
#define __websocket_manager_h

#ifdef __cplusplus
extern "C" {
#endif

enum wm_remote_conn_status {
    WM_REMOTE_DISCONNECTED,
    WM_REMOTE_CONNECTING,
    WM_REMOTE_WAITING_FOR_KEY,
    WM_REMOTE_CONNECTED,
};

typedef enum wm_remote_conn_status wm_remote_conn_status_t;

void websocket_manager_connect_to_remote(const char *hostname, int port);
const char *websocket_manager_get_remote_device_key(void);

#ifdef __cplusplus
}
#endif

#endif
