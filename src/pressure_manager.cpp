#include "pressure_manager.hpp"
#include "m1k-hal.hpp"
#include "esp_log.h"
#include "average.hpp"
#include "esp_log.h"

// TODO: Move these to Config struct.

#define UPDATE_FREQUENCY_MS 10
#define LAST_VALUE_COUNT (2 * (1000 / UPDATE_FREQUENCY_MS))
#define PEAK_AVERAGE_COUNT 3

#define IDLE_SPEED 0x0A

static const char* TAG = "PRESSURE_MANAGER";
static const double pk_threshold = 1.5;
static const long pk_expiry_ms = 3000L;

static MovingAverage<double, PEAK_AVERAGE_COUNT> min_peak;
static MovingAverage<double, PEAK_AVERAGE_COUNT> max_peak;
static MovingAverage<double, LAST_VALUE_COUNT> values;

static long _last_update_ms = 0;
static double _local_peak = 1000;
static bool _rising = false;

static long _low_peak_ms = 0;
static long _high_peak_ms = 0;
static long _pk_pk_ms = 0;

static pressure_manager_seek_status_t _seek_status = PM_SEEK_DISABLED;
static double _seek_target_mean = 0.0;
static int _seek_strokes_remain = 0;
static int _seek_adjust_attempts = 0;
static long _seek_last_auto_ms = 0;

static bool _stop_requested = false;

void _check_seek_pressure(void);
static void _on_rising_peak(void);
static void _on_falling_peak(void);

void pressure_manager_tick(void) {
    long ms = esp_timer_get_time() / 1000;

    if (ms - _seek_last_auto_ms > PM_SEEK_AUTO_INTERVAL_MS) {
        _seek_last_auto_ms = ms;
        if (_seek_status == PM_SEEK_AT_SET_POINT || _seek_status == PM_SEEK_TIMEOUT) {
            _check_seek_pressure();
        }
    }

    if (ms - _last_update_ms < UPDATE_FREQUENCY_MS) {
        return;
    } else {
        _last_update_ms = ms;
    }

    double pressure = m1k_hal_get_pressure_reading();

    if (_rising) {
        if (pressure > _local_peak) {
            _local_peak = pressure;
        } else if (pressure < _local_peak - pk_threshold) {
            _rising = false;
            _pk_pk_ms = ms - _high_peak_ms;
            _high_peak_ms = ms;
            max_peak.insert(_local_peak, ms);
            _on_rising_peak();
        } else {
            // possibly in a peak
        }
    } else {
        if (pressure < _local_peak) {
            _local_peak = pressure;
        } else if (pressure > _local_peak + pk_threshold) {
            _rising = true;
            _low_peak_ms = ms;
            min_peak.insert(_local_peak, ms);
            _on_falling_peak();
        } else {
            // possibly in a peak
        }
    }

    if ((_low_peak_ms != 0 && ms - _low_peak_ms > pk_expiry_ms) &&
        (_high_peak_ms != 0 && ms - _high_peak_ms > pk_expiry_ms)) {
        // we haven't seen a wave in a while so uh i guess it stopped?
        _low_peak_ms = 0;
        _high_peak_ms = 0;
        _local_peak = 1000;
        _rising = false;
        _stop_requested = false;
        _seek_status = PM_SEEK_DISABLED;
        _pk_pk_ms = 0;
        min_peak.clear();
        max_peak.clear();

        // Close air in case seeking:
        m1k_hal_air_stop();
    }
}

void _check_seek_pressure(void) {
    double delta = pressure_manager_get_mean() - _seek_target_mean;

    if (fabs(delta) > PM_SEEK_DELTA_THRESHOLD) {
        ESP_LOGE(TAG, "PM Seek exceeds threshold: %0.2f > %0.2f", delta, PM_SEEK_DELTA_THRESHOLD);

        if (_seek_status != PM_SEEK_CHECK_PRESSURE) {
            _seek_adjust_attempts = PM_SEEK_ATTEMPT_LIMIT;
        }

        _seek_strokes_remain = ceil(fabs(delta) / 4);

        ESP_LOGE(TAG, "Correcting, %d attempts remain...", _seek_adjust_attempts);

        if (delta < PM_SEEK_DELTA_THRESHOLD) {
            _seek_status = PM_SEEK_INCREASE_PRESSURE;
            m1k_hal_air_in();
        } else {
            _seek_status = PM_SEEK_DECREASE_PRESSURE;
            m1k_hal_air_out();
        }
    } else {
        ESP_LOGE(TAG, "PM Seek within threshold: %0.2f <= %0.2f", delta, PM_SEEK_DELTA_THRESHOLD);
        _seek_status = PM_SEEK_AT_SET_POINT;
    }
}

void _on_rising_peak(void) {

}

void _on_falling_peak(void) {
    bool seeking = _seek_status == PM_SEEK_DECREASE_PRESSURE
        || _seek_status == PM_SEEK_INCREASE_PRESSURE;

    // Close Air Valves
    if (seeking && _seek_strokes_remain-- <= 0) {
        _seek_strokes_remain = PM_SEEK_CHECK_STROKE_COUNT;
        _seek_status = PM_SEEK_CHECK_PRESSURE;
        m1k_hal_air_stop();
    }

    // Check Stroke Burndown
    if (_seek_status == PM_SEEK_CHECK_PRESSURE && _seek_strokes_remain-- <= 0) {
        _seek_strokes_remain = 0;
        
        if (_seek_adjust_attempts-- <= 0) {
            _seek_status = PM_SEEK_TIMEOUT;
        } else {
            _check_seek_pressure();
        }
    }

    // Handle Stop Requests Here
    if (_stop_requested) {
        ESP_LOGE(TAG, "Finalizing stop request.");
        _stop_requested = false;
        m1k_hal_set_milker_speed(0x00);
        m1k_hal_hv_power_off();
    }
}

double pressure_manager_get_min_peak(void) {
    return min_peak.avg();
}

double pressure_manager_get_max_peak(void) {
    return max_peak.avg();
}

double pressure_manager_get_amplitude(void) {
    return max_peak.avg() - min_peak.avg();
}

double pressure_manager_get_frequency(void) {
    if (_pk_pk_ms == 0) return 0.0f;
    return 1000.0f / _pk_pk_ms;
}

double pressure_manager_get_mean(void) {
    return min_peak.avg() + (pressure_manager_get_amplitude() / 2);
}

void pressure_manager_request_stop(void) {
    ESP_LOGE(TAG, "Stop requested!");
    if (m1k_hal_hv_is_on()) {
        ESP_LOGE(TAG, "Stop requested (Confirm.)");
        _stop_requested = true;

        if (m1k_hal_get_milker_speed() > IDLE_SPEED) {
            m1k_hal_set_milker_speed(IDLE_SPEED);
        }
    }
}

void pressure_manager_cancel_stop_request(void) {
    _stop_requested = false;
}

bool pressure_manager_is_stop_requested(void) {
    return _stop_requested;
}

void pressure_manager_set_target_mean(double mean) {
    _seek_status = PM_SEEK_AT_SET_POINT;
    _seek_target_mean = mean;
}

double pressure_manager_get_target_mean(void) {
    return _seek_target_mean;
}

pressure_manager_seek_status_t pressure_manager_get_seek_status(void) {
    return _seek_status;
}

void pressure_manager_clear_target_mean(void) {
    _seek_status = PM_SEEK_DISABLED;
    _seek_target_mean = 0.0;
}