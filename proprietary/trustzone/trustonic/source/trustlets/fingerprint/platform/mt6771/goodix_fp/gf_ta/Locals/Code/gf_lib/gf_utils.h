/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_UTILS_H__
#define __GF_UTILS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GF_PERFORMANCE_DUMP_ENABLED (g_sensor.config.support_performance_dump > 0)

#define GF_GET_TIME_DELTA_X(X) \
if (g_sensor.config.support_performance_dump > 0) { \
    X = gf_get_time_delta(); \
}

#define GF_GET_TIME_DELTA() \
if (g_sensor.config.support_performance_dump > 0) { \
    gf_get_time_delta(); \
}

uint32_t gf_get_time_delta(void);

#ifdef __cplusplus
}
#endif

#endif // __GF_UTILS_H__
