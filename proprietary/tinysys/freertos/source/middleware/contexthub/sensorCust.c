/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#ifdef CFG_ALSPS_SUPPORT
#include "cust_alsps.h"
#endif
#ifdef CFG_ACCGYRO_SUPPORT
#include "cust_accGyro.h"
#endif
#ifdef CFG_BAROMETER_SUPPORT
#include "cust_baro.h"
#endif
#ifdef CFG_MAGNETOMETER_SUPPORT
#include "cust_mag.h"
#endif
#ifdef CFG_SAR_SUPPORT
#include "cust_sar.h"
#endif

#define CHECK_NAME(name)    \
({    \
    if(NULL == name) return NULL;    \
})

#define get_per_sensor_cust(name, start, end)    \
({    \
    __typeof__(*start) *_ptr = NULL, *_tmp_ptr = NULL;    \
    for (_tmp_ptr = start; _tmp_ptr < end; _tmp_ptr++) {    \
        if(0 == strcmp(name, _tmp_ptr->name)) {    \
            _ptr = _tmp_ptr;    \
            break;    \
        }    \
    }    \
    _ptr;    \
})
#ifdef CFG_ALSPS_SUPPORT
struct alsps_hw *get_cust_alsps(const char *name)
{
    CHECK_NAME(name);
    extern struct alsps_hw __cust_alsps_start[];
    extern struct alsps_hw __cust_alsps_end[];
    return get_per_sensor_cust(name, __cust_alsps_start, __cust_alsps_end);
}
#endif
#ifdef CFG_ACCGYRO_SUPPORT
struct accGyro_hw *get_cust_accGyro(const char *name)
{
    CHECK_NAME(name);
    extern struct accGyro_hw __cust_accGyro_start[];
    extern struct accGyro_hw __cust_accGyro_end[];
    return get_per_sensor_cust(name, __cust_accGyro_start, __cust_accGyro_end);
}
#endif
#ifdef CFG_BAROMETER_SUPPORT
struct baro_hw *get_cust_baro(const char *name)
{
    CHECK_NAME(name);
    extern struct baro_hw __cust_baro_start[];
    extern struct baro_hw __cust_baro_end[];
    return get_per_sensor_cust(name, __cust_baro_start, __cust_baro_end);
}
#endif
#ifdef CFG_MAGNETOMETER_SUPPORT
struct mag_hw *get_cust_mag(const char *name)
{
    CHECK_NAME(name);
    extern struct mag_hw __cust_mag_start[];
    extern struct mag_hw __cust_mag_end[];
    return get_per_sensor_cust(name, __cust_mag_start, __cust_mag_end);
}
#endif

#ifdef CFG_SAR_SUPPORT
struct sar_hw *get_cust_sar(const char *name)
{
    CHECK_NAME(name);
    extern struct sar_hw __cust_sar_start[];
    extern struct sar_hw __cust_sar_end[];
    return get_per_sensor_cust(name, __cust_sar_start, __cust_sar_end);
}
#endif

