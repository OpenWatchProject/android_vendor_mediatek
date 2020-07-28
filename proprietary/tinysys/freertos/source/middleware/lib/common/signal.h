#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "activity_algorithm.h"

#define SIGNAL_PROTECT_JUMP 2000 // unit:ms
#define SIGNAL_PROTECT_INTERVAL 10 // unit:ms

void get_signal_acc(Activity_Accelerometer_Ms2_Data_t signal_xyz);
void get_signal_baro(Activity_Barometer_Data_t signal_baro);
void check_signal_range(Activity_Accelerometer_Ms2_Data_t* signal_xyz);

#endif
