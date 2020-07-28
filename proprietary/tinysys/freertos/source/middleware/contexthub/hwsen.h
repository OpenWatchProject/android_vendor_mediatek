#ifndef __HWSEN_H__
#define __HWSEN_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>


#define MAX_SENSOR_AXIS_NUM             4
#define MAX_PHYSIC_SENSOR_NUM           6
struct sensorDriverConvert {
    int8_t    sign[MAX_SENSOR_AXIS_NUM];
    uint8_t   map[MAX_SENSOR_AXIS_NUM];
};
int sensorDriverGetConvert(int32_t direction, struct sensorDriverConvert *cvt);
#endif
