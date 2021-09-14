#include "tscode_manager.h"
#include "tscode_capabilities.h"
#include "pressure_manager.hpp"
#include "version.h"
#include "m1k-hal.hpp"
#include "wifi_manager.h"

tscode_command_response_t tscode_callback(tscode_command_t* cmd, char* response, size_t resp_len);

static wifi_ap_config_t _tmp_wifi_config;

void tscode_manager_init(void) {
    _tmp_wifi_config.ssid[0] = '\0';
    _tmp_wifi_config.password[0] = '\0';

    tscode_device_vendor_details_t vendor = {
        .vendor = "Maus-Tec Electronics",
        .device = "Mercury 1000",
        .version = VERSION
    };

    tscode_register_vendor_details(&vendor);

    tscode_cap_channel_t cap = TSCODE_CAP_CHANNEL_DEFAULT;

    // Air valves control:
    cap.capability = TSCODE_CAP_AIR_VALVES;
    tscode_register_channel(&cap);

    // Reciprocating motion for the stroker:
    cap.capability = TSCODE_CAP_RECIPROCATING;
    cap.range.unit = TSCODE_UNIT_PERCENTAGE;
    cap.range.max = 1.0;
    tscode_register_channel(&cap);

    // Support message display
    cap.capability = TSCODE_CAP_MESSAGES;
    cap.range.unit = TSCODE_UNIT_NONE;
    cap.range.max = 40;
    tscode_register_channel(&cap);

    // Conditional stop (return to 0 then stop)
    cap.capability = TSCODE_CAP_CONDITIONAL_STOP;
    tscode_register_channel(&cap);
}

void tscode_manager_tick(void) {
    tscode_process_stream(stdin, stdout, &tscode_callback);
}

tscode_command_response_t tscode_callback(tscode_command_t* cmd, char* response, size_t resp_len) {
    switch (cmd->type) {
    case TSCODE_AIR_IN_OPEN:
        m1k_hal_air_in();
        break;

    case TSCODE_AIR_OUT_OPEN:
        m1k_hal_air_out();
        break;

    case TSCODE_AIR_CLOSE:
        m1k_hal_air_stop();
        break;

    case TSCODE_RECIPROCATING_MOVE: {
        uint8_t speed;
        if (cmd->speed == NULL) {
            speed = 0;
        } else if (cmd->speed->unit == TSCODE_UNIT_PERCENTAGE) {
            speed = 255.0 * cmd->speed->value;
        } else {
            speed = cmd->speed->value;
        }

        if (speed == 0) {
            pressure_manager_request_stop();
        } else {
            m1k_hal_set_milker_speed(speed);
            m1k_hal_hv_power_on();
        }
        break;
    }

    case TSCODE_CONDITIONAL_STOP:
        pressure_manager_request_stop();
        break;

    case TSCODE_HALT_IMMEDIATE:
        m1k_hal_hv_power_off();
        m1k_hal_set_milker_speed(0x00);
        break;

    // Set WiFi SSID
    case __S(170): {
        if (cmd->str != NULL && cmd->str[0] != '\0') {
            printf("WIFI SET SSID: %s\n", cmd->str);
            strncpy(_tmp_wifi_config.ssid, cmd->str, WIFI_SSID_MAX_LEN);
        } else {
            return TSCODE_RESPONSE_FAULT;
        }
        break;
    }

    // Set WiFi Password
    case __S(171): {
        if (cmd->str != NULL && cmd->str[0] != '\0') {
            printf("WIFI SET PASSWORD: %s\n", cmd->str);
            strncpy(_tmp_wifi_config.password, cmd->str, WIFI_PASSWORD_MAX_LEN);
        } else {
            return TSCODE_RESPONSE_FAULT;
        }
        break;
    }

    // Connect to WiFi
    case __S(172): {
        if (!wifi_manager_connect(&_tmp_wifi_config)) {
            return TSCODE_RESPONSE_FAULT;
        }
    }

    default:
        return TSCODE_RESPONSE_NO_CAPABILITY;
    }

    return TSCODE_RESPONSE_OK;
}