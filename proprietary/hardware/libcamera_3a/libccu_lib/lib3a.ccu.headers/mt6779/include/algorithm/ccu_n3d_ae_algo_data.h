#ifndef __CCU_N3D_AE_ALGO_DATA_H__
#define __CCU_N3D_AE_ALGO_DATA_H__

#include "algorithm/ccu_awb_param.h"
#include "algorithm/ccu_AEPlinetable.h"
#include "algorithm/ccu_MTKSyncAe.h"

#define AE_MAX(a, b)  ((a) > (b) ? (a) : (b))
#define AE_MIN(a, b)  ((a) < (b) ? (a) : (b))

typedef struct
{
    MUINT32 u4AEMasterIdx;
    MUINT32 u4SyncGain[4];
    MINT32  i4Regression;
    MINT32  i4SyncMode;
    MINT32  i4EvDiff;
} CCU_N3DAE_STRUCT;

#define CCU_EV_DIFF_RANGE           30
#define CCU_EV_CAL_BASE             4096
#define CCU_REGRESSION_GAIN_BASE    1024
#define CCU_REGRESSION_GAIN_MAX     2048
#define CCU_REGRESSION_GAIN_MIN     512
#define CCU_OFFSET_BASE1000         100
#define CCU_OFFSET_BASE10          1
#define CCU_AESYNC_ISPGAIN_MAX  16384   // 16 x 1024
#define CCU_AESYNC_ISPGAIN_MIN  1024
#define CCU_AESYNC_ISPGAIN_BASE 8


typedef struct
{
    MINT32 SyncNum;
    CCU_SYNC_AE_TUNNING_PARAM_STRUCT SyncAeTuningParam;
    CCU_SYNC_AE_INIT_INPUT_PARAM main_param;
    CCU_SYNC_AE_INIT_INPUT_PARAM sub_param;
}CCU_N3D_AE_INIT_STRUCT;

#endif