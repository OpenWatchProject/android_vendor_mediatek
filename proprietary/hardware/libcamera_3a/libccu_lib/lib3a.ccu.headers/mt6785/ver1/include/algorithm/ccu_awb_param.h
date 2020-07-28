#ifndef _CCU_AWB_PARAM_H
#define _CCU_AWB_PARAM_H

#include "ccu_ext_interface/ccu_types.h"

typedef struct
{
    MUINT8 u2R; // R average of specified AWB window
    MUINT8 u2G; // G average of specified AWB window
    MUINT8 u2B; // B average of specified AWB window
    MUINT8 u2EL; // error number + light source info
} CCU_AWB_MAIN_STAT_2B_T;

typedef union
{
    CCU_AWB_MAIN_STAT_2B_T rMainStat;
} CCU_AWB_WINDOW_2B_T;

// AWB gain
// 3*4 = 12bytes
typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4G; // G gain
    MINT32 i4B; // B gain
} CCU_AWB_GAIN_T;

#endif

