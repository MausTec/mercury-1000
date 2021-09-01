#ifndef __pressure_manager_hpp
#define __pressure_manager_hpp

enum pressure_manager_seek_status {
    PM_SEEK_DISABLED,
    PM_SEEK_INCREASE_PRESSURE,
    PM_SEEK_DECREASE_PRESSURE,
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

#endif