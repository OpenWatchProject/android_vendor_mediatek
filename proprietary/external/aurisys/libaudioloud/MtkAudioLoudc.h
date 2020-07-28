/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
* Description:
*   This file implements Aurisys and Audio Besssound related variables.
*/
#ifndef __MTK_AUDIO_LOUDC_H__
#define __MTK_AUDIO_LOUDC_H__


#include "BesLoudness_HD_exp.h"

#define BLOCK_SIZE 512
#define BLOCK_SIZE_LOW_LATENCY 256
#define DRC_NOISEFILTER_MIN 80 // means -80db/-75db
#define NOISE_FILTER_BASE -256
#define NOISE_FILTER_STEP 5
#define MIN_MULTI_BAND_COUNT 5
#define MAX_MULTI_BAND_COUNT 10
#define MUSIC_UP_BOUND 11   // mMusicCount > MUSIC_UP_BOUND, use MIN_MULTI_BAND_COUNT
#define MUSIC_LOW_BOUND 5   // mMusicCount < MUSIC_LOW_BOUND, use MAX_MULTI_BAND_COUNT


typedef enum {
    BLOUD_STATE_NONE,
    BLOUD_STATE_INIT,
    BLOUD_STATE_OPEN,
    BLOUD_STATE_PROCESS,
} BLOUD_STATUS;

typedef enum {
    BLOUD_PAR_SET_FILTER_TYPE,
    BLOUD_PAR_SET_WORK_MODE,
    BLOUD_PAR_SET_CHANNEL_NUMBER,
    BLOUD_PAR_SET_SAMPLE_RATE,
    BLOUD_PAR_SET_PCM_FORMAT,
    BLOUD_PAR_SET_USE_DEFAULT_PARAM,
    BLOUD_PAR_SET_PREVIEW_PARAM,
    BLOUD_PAR_SET_USE_DEFAULT_PARAM_SUB,
    BLOUD_PAR_SET_PREVIEW_PARAM_SUB,
    BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP,
    BLOUD_PAR_SET_SEP_LR_FILTER,//10
    BLOUD_PAR_SET_STEREO_TO_MONO_MODE,
    BLOUD_PAR_SET_RAMP_UP,
    BLOUD_PAR_SET_USE_DEFAULT_PARAM_FORCE_RELOAD,//bypass cache to load parameter
    BLOUD_PAR_SET_NOISE_FILTER,
    BLOUD_PAR_GET_FILTER_COEF_BUFSIZE,
    BLOUD_PAR_GET_FILTER_COEF,
    BLOUD_PAR_SET_CHANGE_TO_BYPASS,
    BLOUD_PAR_SET_CHANGE_TO_NORMAL,
} BLOUD_PARAMETER;

typedef enum {
    BLOUD_IN_Q1P15_OUT_Q1P15  = 0,   // 16-bit Q1.15  input, 16-bit Q1.15 output
    BLOUD_IN_Q1P31_OUT_Q1P31  = 1,   // 32-bit Q1.31  input, 32-bit Q1.31 output
} BLOUD_PCM_FORMAT;

typedef enum {
    BLOUD_S2M_MODE_NONE      = 0,  // None
    BLOUD_S2M_MODE_ST2MO2ST  = 1,  // Stereo to mono to stereo
} BLOUD_S2M_MODE_ENUM;

typedef enum {
    FILTER_BYPASS_ACF  = 0x1,
    FILTER_BYPASS_HCF  = 0x2,
    FILTER_BYPASS_DRC  = 0x4,
} FILTER_BYPASS_STATUS;

typedef struct {
    uint32_t mTempBufSize; // in byte
    uint32_t mInternalBufSize; // in byte
    char *mpTempBuf;
    char *mpInternalBuf;
} lib_workingbuf_info_t;

typedef struct {
     uint32_t mDrcSwitch;
     FILTER_BYPASS_STATUS mBypassFlag;
     AUDIO_ACF_CUSTOM_PARAM_STRUCT sXmlParam;
     AUDIO_ACF_CUSTOM_PARAM_STRUCT sDrcParam;
} audioloud_lib_param_t;

typedef struct {
    BS_HD_Handle mBloudHandle;
    BLOUD_HD_InitParam mInitParam;  // loudness param
    BLOUD_HD_IIR_Design_Param mParamFormatUse;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT mCachedAudioParam;
    lib_workingbuf_info_t mLibWorkingBuf;
    audioloud_lib_param_t *pLibParamBuf;
    uint32_t mFilterType;
    uint32_t mStreamType;       // To Record the streamType before DRC off
    uint32_t mOutputDevice;
    uint32_t mScenario;
    uint32_t mBypassFlag;
    char mCustScene[65];
    bool bIsEnhaceOn;
    BLOUD_STATUS mState;
    uint32_t id;
} audioloud_lib_handle_t;





#endif // __MTK_AUDIO_LOUDC_H__
