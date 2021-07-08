#include "pressure_manager.hpp"
#include "m1k-hal.hpp"
#include "esp_log.h"
#include "average.hpp"

#define UPDATE_FREQUENCY_MS 10
#define LAST_VALUE_COUNT (2 * (1000 / UPDATE_FREQUENCY_MS))
#define PEAK_AVERAGE_COUNT 3

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

void pressure_manager_tick(void) {
    long ms = esp_timer_get_time() / 1000;
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
        _pk_pk_ms = 0;
        min_peak.clear();
        max_peak.clear();
    }

    if (false) {
        Serial.printf("%.2f,%.1f,%.1f,%.1f\n",
            pressure,
            pressure_manager_get_frequency(),
            pressure_manager_get_min_peak(),
            pressure_manager_get_max_peak()
        );
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