#include "ui.hpp"
#include "pages.hpp"
#include "menus.hpp"
#include "graphics.hpp"
#include "pressure_manager.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include <math.h>
#include "cstring"

#include "images/milker_icon_0.h"
#include "images/milker_icon_1.h"
#include "images/milker_icon_2.h"
#include "images/milker_icon_3.h"

using namespace UI;
static const char* TAG = "PAGE:HOME";

#define UPDATE_PERIOD_MS (1000 / 30)
#define DOUBLE_CLICK_DELAY_MS 500

enum current_control_item {
    HOME_CONTROL_ITEM_SPEED,
    HOME_CONTROL_ITEM_PRESSURE
};

typedef enum current_control_item current_control_item_t;

static struct {
    int speed = 0;
    int icon_step = 2;
    double pressure = 0;
    long last_update_ms = 0;
    long last_click_ms = 0;
    double frequency = 0;
    double peak_max = 0;
    double peak_min = 0;
    bool stop_requested = false;
    current_control_item_t ctrl_item = HOME_CONTROL_ITEM_SPEED;
    m1k_hal_air_direction_t air_direction = M1K_HAL_AIR_CLOSED;
    long last_icon_ms = 0;
    long icon_step_ms = 0;
} state;

static int _speed_count = 20;

static graphics_image_t* MILKER_ICONS[6] = {
    &MILKER_ICON_0,
    &MILKER_ICON_1,
    &MILKER_ICON_2,
    &MILKER_ICON_3,
    &MILKER_ICON_2,
    &MILKER_ICON_1,
};

// label top = 0, bototm = 4
// large top = 7, bottom = 17
// bar top = 19, bottom = 32

static void draw_pressure_graph(u8g2_t* display, int min, int max, int lmark, int rmark, double value) {
    auto left = m1k_hal_get_display_left() + 23;
    auto width = m1k_hal_get_display_width() - 23;
    auto top = 19;

    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }

    if (lmark < min) {
        lmark = min;
    } else if (lmark > max) {
        lmark = max;
    }

    if (rmark < min) {
        rmark = min;
    } else if (rmark > max) {
        rmark = max;
    }

    // Down Arrow
    u8g2_DrawVLine(display, left + 2, top, 4);
    u8g2_DrawTriangle(display, left, top + 4, left + 5, top + 4, left + 2, top + 7);

    // Rect
    auto bar_left = left + 7;
    auto bar_width = width - 16;
    auto bar_center_x = bar_left + (bar_width / 2);
    double fill_space = floor(bar_width / 2.0);

    u8g2_DrawFrame(display, bar_left, top, bar_width, 7);
    u8g2_DrawVLine(display, bar_center_x, top, 7);

    if (value < 0) {
        double fill_width = floor(fabs((double) value / min) * fill_space);
        u8g2_DrawBox(display, bar_center_x - fill_width, top + 2, fill_width, 3);
    } else {
        double fill_width = floor(fabs((double) value / max) * fill_space);
        u8g2_DrawBox(display, bar_center_x, top + 2, fill_width + 1, 3);
    }

    if (lmark < 0) {
        double fill_width = floor(fabs((double) lmark / min) * fill_space);
        u8g2_DrawBox(display, bar_center_x - fill_width, top + 1, 1, 5);
    } else {
        double fill_width = floor(fabs((double) lmark / max) * fill_space);
        u8g2_DrawBox(display, bar_center_x + (fill_width - 1), top + 1, 1, 5);
    }

    if (rmark < 0) {
        double fill_width = floor(fabs((double) rmark / min) * fill_space);
        u8g2_DrawBox(display, bar_center_x - fill_width, top + 1, 1, 5);
    } else {
        double fill_width = floor(fabs((double) rmark / max) * fill_space);
        u8g2_DrawBox(display, bar_center_x + (fill_width - 1), top + 1, 1, 5);
    }


    // Up Arrow
    auto rtri_left = left + width - 7;
    u8g2_DrawVLine(display, rtri_left + 2, top + 3, 4);
    u8g2_DrawTriangle(display, rtri_left - 1, top + 3, rtri_left + 5, top + 3, rtri_left + 2, top - 1);

    // Lower Text
    u8g2_SetFont(display, u8g2_font_4x6_mf);
    const char* unit_center = "0KPA";
    auto unit_center_x = bar_center_x - (u8g2_GetStrWidth(display, unit_center) / 2) + 1;
    auto unit_top = top + 13;
    u8g2_DrawStr(display, unit_center_x, unit_top, unit_center);

    char unit_left[10] = "";
    snprintf(unit_left, 10, "%d", min);
    u8g2_DrawStr(display, bar_left, unit_top, unit_left);

    char unit_right[10] = "";
    snprintf(unit_right, 10, "%d", max);
    auto unit_right_x = bar_left + bar_width - u8g2_GetStrWidth(display, unit_right) + 1;
    u8g2_DrawStr(display, unit_right_x, unit_top, unit_right);
}

static void draw_speed(u8g2_t* display, int speed, bool selected) {
    auto left = m1k_hal_get_display_left() + 23;
    auto width = m1k_hal_get_display_width() - 23;
    auto center = left + width / 2;

    u8g2_SetFont(display, u8g2_font_4x6_mf);
    const char* label = "SPEED";
    auto label_width = u8g2_GetStrWidth(display, label);
    u8g2_DrawStr(display, center - floor(label_width / 2), 5, label);

    u8g2_SetFont(display, u8g2_font_7x14B_tf);

    if (speed > _speed_count && !state.stop_requested) {
        speed = _speed_count;
        u8g2_DrawStr(display, center + 9, 15, "+");
    }

    char value[4] = "";
    
    if (state.stop_requested) {
        value[0] = 'S';
        value[1] = '\0';
    } else {
        snprintf(value, 4, "%d", state.stop_requested ? 0 : speed);
    }

    auto value_width = u8g2_GetStrWidth(display, value);
    u8g2_DrawStr(display, center - floor(value_width / 2) - 1, 17, value);

    if (selected) {
        int triangle_top = 9;
        int triangle_tip = 10;

        if (speed > 0 && !state.stop_requested) {
            u8g2_DrawTriangle(display, center - triangle_tip, triangle_top, center - triangle_tip, triangle_top + 6, center - (triangle_tip + 3), triangle_top + 3);
        }

        if (speed < _speed_count) {
            u8g2_DrawTriangle(display, center + triangle_tip - 1, triangle_top, center + triangle_tip - 1, triangle_top + 6, center + (triangle_tip + 2), triangle_top + 3);
        }
    }
}

static void draw_depth(u8g2_t* display, int pressure_delta, bool selected) {
    const auto left = m1k_hal_get_display_left() + 23 + 7;

    u8g2_SetFont(display, u8g2_font_4x6_mf);
    const char* label = "DEPTH";
    auto label_width = u8g2_GetStrWidth(display, label);
    u8g2_DrawStr(display, left + 5, 5, label);

    if (state.air_direction == M1K_HAL_AIR_CLOSED) {
        const int plus_minus_top = 10;
        u8g2_DrawLine(display, left + 1, plus_minus_top, left + 1, plus_minus_top + 2);
        u8g2_DrawLine(display, left, plus_minus_top + 1, left + 2, plus_minus_top + 1);
        u8g2_DrawLine(display, left, plus_minus_top + 5, left + 2, plus_minus_top + 5);
    }

    u8g2_SetFont(display, u8g2_font_7x14B_tf);
    const int value_center = left + (label_width / 2) + 5;
    char value[4] = "";

    if (state.air_direction == M1K_HAL_AIR_CLOSED) {
        snprintf(value, 4, "%d", pressure_delta);
    } else if (state.air_direction == M1K_HAL_AIR_OUT) {
        strncpy(value, "OUT", 4);
    } else {
        strncpy(value, "IN", 4);
    }

    int value_width = u8g2_GetStrWidth(display, value);
    u8g2_DrawStr(display, value_center - floor(value_width / 2) - 1, 17, value);
}

static void draw_frequency(u8g2_t* display, double frequency) {
    char label[16] = "STR/s";
    u8g2_SetFont(display, u8g2_font_4x6_tf);
    int left = m1k_hal_get_display_left() + (m1k_hal_get_display_width()
        - u8g2_GetStrWidth(display, label) - 0);
    int top = 17;
    u8g2_DrawStr(display, left, top, label);

    // decimal part first because this stupid wide space and period
    int decimal = frequency * 10;
    snprintf(label, 16, "%d", decimal % 10);
    left -= u8g2_GetStrWidth(display, label) + 2;
    u8g2_DrawStr(display, left, top, label);

    // fuck it we'll just put a pixel here
    u8g2_DrawPixel(display, left - 2, top - 1);

    // okay and the rest
    snprintf(label, 16, "%d", (int) floor(frequency));
    left -= u8g2_GetStrWidth(display, label) + 3;
    u8g2_DrawStr(display, left, top, label);
}

static void render(u8g2_t* display, Page* page) {
    auto milker_icon = MILKER_ICONS[state.icon_step];

    u8g2_SetDrawColor(display, 1);
    graphics_draw_image(0, 0, milker_icon);
    draw_pressure_graph(display, -30, 15, state.peak_min, state.peak_max, state.pressure);
    draw_speed(display, state.speed, state.ctrl_item == HOME_CONTROL_ITEM_SPEED);
    draw_depth(display, state.peak_max - state.peak_min, state.ctrl_item == HOME_CONTROL_ITEM_PRESSURE);
    draw_frequency(display, state.frequency);
}

static int get_speed(void) {
    return sqrt(m1k_hal_get_milker_speed()) / 0.6;
}

static void set_speed(int speed) {
    if (speed == 0) {
        pressure_manager_request_stop();
        state.stop_requested = true;
    } else {
        state.stop_requested = false;
        pressure_manager_cancel_stop_request();
        m1k_hal_set_milker_speed(speed);
        m1k_hal_hv_power_on();
    }
}

static void loop(Page* page) {
    long ms = esp_timer_get_time() / 1000;

    if (ms - state.last_update_ms > UPDATE_PERIOD_MS) {
        state.last_update_ms = ms;
        state.pressure = pressure_manager_get_mean();
        state.frequency = pressure_manager_get_frequency();
        state.peak_max = pressure_manager_get_max_peak();
        state.peak_min = pressure_manager_get_min_peak();
        state.icon_step = fmin(4.0, fmax(0.0, (m1k_hal_get_pressure_reading() + 25.0) / 10.0));
        state.stop_requested = pressure_manager_is_stop_requested();
        state.speed = get_speed();
        state.air_direction = m1k_hal_air_get_direction();

        page->render();
    }
}

static void exit(Page* page) {
    set_speed(0x00);
}

static void enter(Page* page) {
    state.speed = m1k_hal_get_milker_speed();
}

static void encoder_change(int difference, Page* page) {
    switch (state.ctrl_item) {
        case HOME_CONTROL_ITEM_SPEED:
        default: {
            if (!state.stop_requested || difference >= 0) {
                state.speed += difference;
                if (state.speed < 1) state.speed = 0;
                if (state.speed > _speed_count) state.speed = _speed_count;

                int speed = ceil(pow(state.speed * 0.6, 2));
                ESP_LOGE(TAG, "Set speed: %d", speed);
                set_speed(speed);
            }
            break;
        }
    }
    
    page->render();
}

static void on_button(m1k_hal_button_t button, m1k_hal_button_evt_t evt, Page *page) {
    if (button == M1K_HAL_BUTTON_MENU) {
        if (evt == M1K_HAL_BUTTON_EVT_PRESS) {
            state.ctrl_item = HOME_CONTROL_ITEM_SPEED;
            state.speed = 0;
            set_speed(0);
        } else if (evt == M1K_HAL_BUTTON_EVT_HOLD) {
            ui_open_menu(&Menus::MainMenu);
        }
    }

    if (button == M1K_HAL_BUTTON_AIRIN) {
        if (evt == M1K_HAL_BUTTON_EVT_DOWN) {
            ESP_LOGE(TAG, "AIR IN");
            m1k_hal_air_in();
        }

        if (evt == M1K_HAL_BUTTON_EVT_UP) {
            ESP_LOGE(TAG, "AIR STOP");
            m1k_hal_air_stop();
        }
    }

    if (button == M1K_HAL_BUTTON_AIROUT) {
        if (evt == M1K_HAL_BUTTON_EVT_DOWN) {
            ESP_LOGE(TAG, "AIR OUT");
            m1k_hal_air_out();
        }

        if (evt == M1K_HAL_BUTTON_EVT_UP) {
            ESP_LOGE(TAG, "AIR STOP");
            m1k_hal_air_stop();
        }
    }

    page->render();
}

static page_config_t config = {
    .enter_cb = &enter,
    .render_cb = &render,
    .loop_cb = &loop,
    .exit_cb = &exit,
    .button_cb = &on_button,
    .encoder_cb = &encoder_change,
};

Page Pages::Home(&config);