#ifndef __CCU_N3D_AE_ALGO_DATA_H__
#define __CCU_N3D_AE_ALGO_DATA_H__

#include "algorithm/ccu_awb_param.h"
#include "algorithm/ccu_AEPlinetable.h"
#include "algorithm/ccu_MTKSyncAe.h"

#define CCU_N3DAE_SUPPORT_SENSORS 4
#define AE_MAX(a, b)  (a) > (b) ? (a) : (b)
#define AE_MIN(a, b)  (a) < (b) ? (a) : (b)


typedef struct
{
    int nBV;
    int nTblIdx;
    CCU_strEvPline *pCurrentTable;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MINT32        i4MaxBV;
    MINT32        i4MinBV;
} CCU_strLimit;


typedef struct
{
    MUINT32 u4AEMasterIdx;
    MUINT32 u4SyncGain[4];
    CCU_strLimit    Upper; //high light
    CCU_strLimit    Lower; //low light
    CCU_AWB_GAIN_T  CalGain;
    MINT32  i4CalYGain;
    MINT32  i4CalOffset;
    MINT32  i4RegrOffset;

    // Debug
    MINT32  i4DumpStat;
    MINT32  i4DumpTable;
    MINT32  i4Regression;
    MINT32  i4Force;
    MINT32  i4ForceMode;
    MINT32  i4SyncMode;
    MINT32  i4EvDiff;
    MINT32  i4ForceExposureTime;
} CCU_N3DAE_STRUCT;

#define CCU_EV_DIFF_RANGE           30
#define CCU_EV_CAL_BASE             4096
#define CCU_REGRESSION_GAIN_MAX     2048
#define CCU_REGRESSION_GAIN_MIN     512
#define CCU_OFFSET_BASE1000         100
#define CCU_OFFSET_BASE10          1

#define CCU_MAX_MAPPING_POINT 20


typedef struct
{
    CCU_SYNC_AE_SCENARIO_ENUM SyncScenario;
    MINT32 SyncNum;
    CCU_SYNC_AE_TUNNING_PARAM_STRUCT SyncAeTuningParam;
    CCU_SYNC_AE_INIT_INPUT_PARAM main_param;
    CCU_SYNC_AE_INIT_INPUT_PARAM sub_param;
}CCU_N3D_AE_INIT_STRUCT;

//8bytes
typedef struct
{
    MUINT32 x;
    MUINT32 y;
}CCU_SYNC_AE_AAO_AXIS;

//20*8*2 = 320bytes
typedef struct
{
    CCU_SYNC_AE_AAO_AXIS Main[CCU_MAX_MAPPING_POINT];
    CCU_SYNC_AE_AAO_AXIS Main2[CCU_MAX_MAPPING_POINT];
}CCU_SYNC_AE_AAO_MAPPING_INFO_STRUCT;

#endif