#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <seos.h>
#include <util.h>
#include "mtk_overlay_init.h"

typedef int (*autoDetectFunc)(void);

#define SENSOR_OVERLAY_REMAP(name, label)                      \
do {                                                           \
    int ret = 0;                                               \
    void *ptr = NULL;                                          \
    autoDetectFunc auto_detect;                                \
                                                               \
    ptr = TINYSYS_OVERLAY_SELECT(name);                        \
    auto_detect = (autoDetectFunc)ptr;                         \
    ret = auto_detect();                                       \
    if (ret < 0) {                                             \
        osLog(LOG_ERROR, "%s overlay remap fail\n", #name);    \
    } else {                                                   \
        osLog(LOG_ERROR, "%s overlay remap success\n", #name); \
        goto label;                                            \
    }                                                          \
} while(0)

#define ACC_GYRO_OVERLAY_REMAP_START
#define ACC_GYRO_OVERLAY_REMAP_END accGyroEnd:
#define ACC_GYRO_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, accGyroEnd)

#define MAG_OVERLAY_REMAP_START
#define MAG_OVERLAY_REMAP_END magEnd:
#define MAG_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, magEnd)

#define ALSPS_OVERLAY_REMAP_START
#define ALSPS_OVERLAY_REMAP_END alspsEnd:
#define ALSPS_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, alspsEnd)

#define BARO_OVERLAY_REMAP_START
#define BARO_OVERLAY_REMAP_END baroEnd:
#define BARO_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, baroEnd)

void accGyroOverlayRemap(void)
{
ACC_GYRO_OVERLAY_REMAP_START
    ACC_GYRO_OVERLAY_REMAP(lsm6ds3);
    ACC_GYRO_OVERLAY_REMAP(bmi160);
    ACC_GYRO_OVERLAY_REMAP(lis3dh);
    ACC_GYRO_OVERLAY_REMAP(lsm6dsm);
    ACC_GYRO_OVERLAY_REMAP(lis2hh12);
ACC_GYRO_OVERLAY_REMAP_END

    return;
}
void magOverlayRemap(void)
{
MAG_OVERLAY_REMAP_START
    MAG_OVERLAY_REMAP(akm09915);
    MAG_OVERLAY_REMAP(akm09918);
MAG_OVERLAY_REMAP_END

    return;
}
void alspsOverlayRemap(void)
{
ALSPS_OVERLAY_REMAP_START
    ALSPS_OVERLAY_REMAP(cm36558);
ALSPS_OVERLAY_REMAP_END

    return;
}
void baroOverlayRemap(void)
{
BARO_OVERLAY_REMAP_START
    BARO_OVERLAY_REMAP(bmp280);
BARO_OVERLAY_REMAP_END

    return;
}

