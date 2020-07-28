#ifndef PQCZTUNING_H
#define PQCZTUNING_H

#include <stdio.h>
#include <stdlib.h>

#include "feature_Adaptive.h"
#include "feature_cmparam.h"
#define CZ_ANDROID_PLATFORM

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed int int32_t;

struct RszSWTuningReg
{
    int ultraResEnable;
    int IBSE_clip_ratio;
    int IBSE_clip_thr;
    int IBSE_gaincontrol_gain;
    int tapAdaptSlope;
    int dynIBSE_gain;
};

struct RszHWTuningReg
{
    int IBSE_gaincontrol_coring_value;
    int IBSE_gaincontrol_coring_thr;
    int IBSE_gaincontrol_coring_zero;
    int IBSE_gaincontrol_softlimit_ratio;
    int IBSE_gaincontrol_bound;
    int IBSE_gaincontrol_limit;
    int IBSE_gaincontrol_softcoring_gain;
};

struct DSHWTuningReg
{
    int tdshp_en;
    int tdshp_gain_mid;
    int tdshp_gain_high;
    int tdshp_softcoring_gain;

    int tdshp_coring_thr;
    int tdshp_coring_zero;
    int tdshp_gain;
    int tdshp_limit_ratio;
    int tdshp_limit;
    int tdshp_bound;
    int tdshp_coring_value;

    int tdshp_clip_en;
    int tdshp_clip_ratio;
    int tdshp_clip_thr;

    int pbc1_gain;
    int pbc1_lpf_gain;

    int pbc2_gain;
    int pbc2_lpf_gain;

    int pbc3_gain;
    int pbc3_lpf_gain;

    int edf_flat_gain;
    int edf_detail_gain;
    int edf_edge_gain;
};

struct ClzTuningReg
{
    RszSWTuningReg  ClzRszSWReg;
    RszHWTuningReg  ClzRszHWReg;
    DSHWTuningReg  ClzDSHWReg;
};

struct NvramClzTable
{
    int ZOOM[6];
    int ISO[11];
    ClzTuningReg TuningReg[50];
};

struct NvramClzTable_mt6779
{
    int ZOOM[6];
    int ISO[11];
    int defaultUpTable;
    int defaultDownTable;
    int switchRatio6Tap6nTap;
    int switchRatio6nTapAcc;
    ClzTuningReg TuningReg[50];
};

void CZCalRegRef(int CalValue, int size, uint32_t* RefArray, int* RefValue);
void CZRegInterpolation(int PreRef, int PosRef, int CurrentValue, void *PreInput, void *PosInput, int Size, void *Result);

void CZTuning(uint32_t ISOSpeed, uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight, void* NvRamTable, ClzTuningReg* ResultBuffer);

#endif

