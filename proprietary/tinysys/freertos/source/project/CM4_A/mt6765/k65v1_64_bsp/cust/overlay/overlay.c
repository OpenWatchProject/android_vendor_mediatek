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

#define ALSPS_SECONDARY_OVERLAY_REMAP_START
#define ALSPS_SECONDARY_OVERLAY_REMAP_END alspsSecondaryEnd:
#define ALSPS_SECONDARY_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, alspsSecondaryEnd)

#define BARO_OVERLAY_REMAP_START
#define BARO_OVERLAY_REMAP_END baroEnd:
#define BARO_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, baroEnd)


#if 0
/******************************************************************************
* Overlay region 5 (counting from 0).
******************************************************************************/
#define REGION5_OVERLAY_REMAP_START
#define REGION5_OVERLAY_REMAP_END region5End:
#define REGION5_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, region5End)

/******************************************************************************
* Overlay region 6 (counting from 0).
******************************************************************************/
#define REGION6_OVERLAY_REMAP_START
#define REGION6_OVERLAY_REMAP_END region6End:
#define REGION6_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, region6End)

/******************************************************************************
* Overlay region 7 (counting from 0).
******************************************************************************/
#define REGION7_OVERLAY_REMAP_START
#define REGION7_OVERLAY_REMAP_END region7End:
#define REGION7_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, region7End)

/******************************************************************************
* Overlay region 8 (counting from 0).
******************************************************************************/
#define REGION8_OVERLAY_REMAP_START
#define REGION8_OVERLAY_REMAP_END region8End:
#define REGION8_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, region8End)

/******************************************************************************
* Overlay region 9 (counting from 0).
******************************************************************************/
#define REGION9_OVERLAY_REMAP_START
#define REGION9_OVERLAY_REMAP_END region9End:
#define REGION9_OVERLAY_REMAP(name) SENSOR_OVERLAY_REMAP(name, region9End)
#endif  // 0


/******************************************************************************
******************************************************************************/
void accGyroOverlayRemap(void)
{
ACC_GYRO_OVERLAY_REMAP_START
    ACC_GYRO_OVERLAY_REMAP(lis2hh12);
ACC_GYRO_OVERLAY_REMAP_END

    return;
}


/******************************************************************************
******************************************************************************/
void magOverlayRemap(void)
{
MAG_OVERLAY_REMAP_START
    MAG_OVERLAY_REMAP(akm09918);
MAG_OVERLAY_REMAP_END

    return;
}


/******************************************************************************
******************************************************************************/
void alspsOverlayRemap(void)
{
ALSPS_OVERLAY_REMAP_START
    ALSPS_OVERLAY_REMAP(cm36558);
    ALSPS_OVERLAY_REMAP(fakeAls);
ALSPS_OVERLAY_REMAP_END

    return;
}


/******************************************************************************
******************************************************************************/
void alspsSecondaryOverlayRemap(void)
{
    ALSPS_SECONDARY_OVERLAY_REMAP_START
    ALSPS_SECONDARY_OVERLAY_REMAP(fakePs);
    ALSPS_SECONDARY_OVERLAY_REMAP_END
    return;
}


/******************************************************************************
******************************************************************************/
void baroOverlayRemap(void)
{
BARO_OVERLAY_REMAP_START
    BARO_OVERLAY_REMAP(bmp280);
BARO_OVERLAY_REMAP_END

    return;
}


#if 0
/******************************************************************************
******************************************************************************/
void region5_OverlayRemap(void)
{
REGION5_OVERLAY_REMAP_START
    REGION5_OVERLAY_REMAP(rg5ap1);
    REGION5_OVERLAY_REMAP(rg5ap2);
REGION5_OVERLAY_REMAP_END

    return;
}


/******************************************************************************
******************************************************************************/
void region6_OverlayRemap(void)
{
REGION6_OVERLAY_REMAP_START
    REGION6_OVERLAY_REMAP(rg6ap1);
    REGION6_OVERLAY_REMAP(rg6ap2);
REGION6_OVERLAY_REMAP_END

    return;
}


/******************************************************************************
******************************************************************************/
void region7_OverlayRemap(void)
{
REGION7_OVERLAY_REMAP_START
    REGION7_OVERLAY_REMAP(rg7ap1);
    REGION7_OVERLAY_REMAP(rg7ap2);
REGION7_OVERLAY_REMAP_END

    return;
}


/******************************************************************************
******************************************************************************/
void region8_OverlayRemap(void)
{
REGION8_OVERLAY_REMAP_START
    REGION8_OVERLAY_REMAP(rg8ap1);
    REGION8_OVERLAY_REMAP(rg8ap2);
REGION8_OVERLAY_REMAP_END

    return;
}


/******************************************************************************
******************************************************************************/
void region9_OverlayRemap(void)
{
REGION9_OVERLAY_REMAP_START
    REGION9_OVERLAY_REMAP(rg9ap1);
    REGION9_OVERLAY_REMAP(rg9ap2);
REGION9_OVERLAY_REMAP_END

    return;
}
#endif  // 0

