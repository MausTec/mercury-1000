#ifndef __websocket_manager_h
#define __websocket_manager_h

#ifdef __cplusplus
extern "C" {
#endif

#define MAUS_LINK_REMOTE_HOST "link.maustec.net"
#define MAUS_LINK_DEFAULT_PROTOCOL "wss"

enum wm_remote_conn_status {
    WM_REMOTE_DISCONNECTED = (1 << 0),
    WM_REMOTE_CONNECTING = (1 << 1),
    WM_REMOTE_WAITING_FOR_KEY = (1 << 2),
    WM_REMOTE_CONNECTED = (1 << 3),
};

typedef enum wm_remote_conn_status wm_remote_conn_status_t;

wm_remote_conn_status_t websocket_manager_wait_for_status(int status, unsigned long wait_ms);
wm_remote_conn_status_t websocket_manager_get_status(void);
void websocket_manager_connect_to_remote(const char *hostname, int port);
const char *websocket_manager_get_remote_device_key(void);
void websocket_manager_register_device_key(const char *remote_device_key);

#ifdef __cplusplus
}
#endif

#endif
