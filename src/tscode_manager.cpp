#include "tscode_manager.h"
#include "tscode_capabilities.h"
#include "pressure_manager.hpp"
#include "version.h"
#include "m1k-hal.hpp"
#include "update_manager.h"
#include <driver/i2c.h>
#include "config.hpp"
#include "wifi_manager.h"
#include "ui.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cstring>

#define I2C_EXT_NUM                 I2C_NUM_1
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_FREQ_HZ          10000
#define I2C_TIMEOUT_MS              1000
#define I2C_ACK_CHECK_EN            0x1
#define I2C_ACK_CHECK_DIS           0x0

tscode_command_response_t tscode_callback(tscode_command_t* cmd, char* response, size_t resp_len);

void tscode_manager_init(void) {
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

    // For now, just set direction and manually run I2C commands
    m1k_hal_set_accessory_mode(M1K_HAL_ACCESSORY_SLAVE);
}

void tscode_manager_tick(void) {
    // Process Serial Commands
    tscode_process_stream(stdin, stdout, &tscode_callback);

    // Process I2C Commands
    static char i2c_buffer[256] = "";
    static size_t i2c_buffer_cursor = 0;

    while (0 < i2c_slave_read_buffer(I2C_EXT_NUM, (uint8_t*) i2c_buffer + i2c_buffer_cursor, 1, 0)) {
        char c = i2c_buffer[i2c_buffer_cursor];

        if (c == '\n') {
            char i2c_tx_buffer[120] = "";
            tscode_process_buffer(i2c_buffer, &tscode_callback, i2c_tx_buffer, 119);
            printf("TSCode from I2C: %s\n", i2c_tx_buffer);
            
            i2c_buffer[0] = '\0';
            i2c_buffer_cursor = 0;

            i2c_reset_tx_fifo(I2C_EXT_NUM);
            i2c_slave_write_buffer(I2C_EXT_NUM, (uint8_t*) i2c_tx_buffer, strlen(i2c_tx_buffer), 0);
        } else {
            i2c_buffer_cursor++;
        }
    }

    // Process BLE Commands
}

tscode_command_response_t tscode_callback(tscode_command_t* cmd, char* response, size_t resp_len) {
    // FIXME: this is being cast to an int for now to avoid erroring on non-enumerated custom codes.
    //        all custom codes should be removed and handled elsewhere as a standard command
    switch ((int)cmd->type) {
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

    case TSCODE_DISPLAY_MESSAGE:
        ui_toast(cmd->str, 3000, UI_TOAST_NOFLAG);
        break;

    // Set WiFi SSID
    case __S(170): {
        if (cmd->str != NULL && cmd->str[0] != '\0') {
            printf("WIFI SET SSID: %s\n", cmd->str);
            strncpy(Config.wifi_ssid, cmd->str, 31);
        } else {
            return TSCODE_RESPONSE_FAULT;
        }
        break;
    }

    // Set WiFi Password
    case __S(171): {
        if (cmd->str != NULL && cmd->str[0] != '\0') {
            printf("WIFI SET PASSWORD: %s\n", cmd->str);
            strncpy(Config.wifi_key, cmd->str, 63);
        } else {
            return TSCODE_RESPONSE_FAULT;
        }
        break;
    }

    // Connect to WiFi
    case __S(172): {
        Config.wifi_on = true;
        wifi_manager_connect_to_ap(Config.wifi_ssid, Config.wifi_key);
        printf("connecting to wifi");

        int attempts = 0;
        while (wifi_manager_get_status() != WIFI_MANAGER_CONNECTED) {
            if (attempts++ > 100) {
                printf("failed\n");
                return TSCODE_RESPONSE_FAULT;
            }
            printf(".");
            vTaskDelay(100 * portTICK_PERIOD_MS);
        }

        printf("ok\n");
        printf("IP: %s\n", wifi_manager_get_local_ip());
        config_save_to_nvfs(CONFIG_DEFAULT_FILE, &Config);
        break;
    }

    // Update from Web
    case __S(173): {
        esp_err_t err = update_manager_update_from_web();
        if (err != ESP_OK) {
            printf("update error: %s\n", esp_err_to_name(err));
            return TSCODE_RESPONSE_FAULT;
        } else {
            printf("Updates complete.\nRestarting...");
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            esp_restart();

            for (;;) {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
        break;
    }

    // Set PWM Freq
    case __S(174): {
        if (cmd->str != NULL && cmd->str[0] != '\0') {
            m1k_hal_set_drive_freq(atoi(cmd->str));
        } else {
            return TSCODE_RESPONSE_FAULT;
        }
        break;
    }

    case __S(175): {
        double mean = pressure_manager_get_mean();
        pressure_manager_set_target_mean(mean);
        break;
    }

    default:
        return TSCODE_RESPONSE_NO_CAPABILITY;
    }

    return TSCODE_RESPONSE_OK;
}