#ifndef __pressure_manager_hpp
#define __pressure_manager_hpp

#define PM_SEEK_ATTEMPT_LIMIT 5
#define PM_SEEK_CHECK_STROKE_COUNT 2
#define PM_SEEK_ADJUST_STROKE_COUNT 2
#define PM_SEEK_AUTO_INTERVAL_MS 15000
#define PM_SEEK_DELTA_THRESHOLD 0.5

enum pressure_manager_seek_status {
    PM_SEEK_DISABLED,
    PM_SEEK_INCREASE_PRESSURE,
    PM_SEEK_DECREASE_PRESSURE,
    PM_SEEK_CHECK_PRESSURE,
    PM_SEEK_AT_SET_POINT,
    PM_SEEK_TIMEOUT,
};

typedef enum pressure_manager_seek_status pressure_manager_seek_status_t;

void pressure_manager_tick(void);

double pressure_manager_get_min_peak(void);
double pressure_manager_get_max_peak(void);
double pressure_manager_get_amplitude(void);
double pressure_manager_get_frequency(void);
double pressure_manager_get_mean(void);

void pressure_manager_set_target_mean(double mean);
double pressure_manager_get_target_mean(void);
pressure_manager_seek_status_t pressure_manager_get_seek_status(void);
void pressure_manager_clear_target_mean(void);

void pressure_manager_request_stop(void);
void pressure_manager_cancel_stop_request(void);
bool pressure_manager_is_stop_requested(void);

#endif