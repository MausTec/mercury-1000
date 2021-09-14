#ifndef __wifi_manager_h
#define __wifi_manager_h

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_SSID_MAX_LEN 31
#define WIFI_PASSWORD_MAX_LEN 63

struct wifi_ap_config {
    char ssid[WIFI_SSID_MAX_LEN + 1];
    char password[WIFI_PASSWORD_MAX_LEN + 1];
};

typedef struct wifi_ap_config wifi_ap_config_t;

void wifi_manager_init(void);
int wifi_manager_connect(wifi_ap_config_t *config);

#ifdef __cplusplus
}
#endif

#endif
