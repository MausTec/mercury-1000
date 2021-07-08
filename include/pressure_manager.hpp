#ifndef __pressure_manager_hpp
#define __pressure_manager_hpp

void pressure_manager_tick(void);

double pressure_manager_get_min_peak(void);
double pressure_manager_get_max_peak(void);
double pressure_manager_get_amplitude(void);
double pressure_manager_get_frequency(void);
double pressure_manager_get_mean(void);

#endif