#pragma once
#ifndef _FLICKER_TYPE_H_
#define _FLICKER_TYPE_H_
#include "stdint.h"

#define HAL_FLICKER_AUTO_50HZ    50
#define HAL_FLICKER_AUTO_60HZ    60
#define HAL_FLICKER_AUTO_OFF     0

typedef struct FlkEISVector
{
    int32_t vx[16];
    int32_t vy[16];
} FlkEISVector;

typedef struct FlkSensorInfo
{
    uint32_t pixel_clock;
    uint32_t full_line_width;
} FlkSensorInfo;

typedef struct FlkAEInfo
{
    uint32_t preview_shutter_value;
} FlkAEInfo;

typedef enum FlkStatus { INCONCLUSIVE, FK000, FK100, FK120 } FlkStatus;           // inconclusive, no flicker, 100 Hz, 120 Hz

#endif
