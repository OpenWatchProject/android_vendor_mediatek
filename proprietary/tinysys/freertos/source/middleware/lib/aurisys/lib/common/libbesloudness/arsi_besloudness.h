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
#ifndef __ARSI_BESLOUDNESS_H__
#define __ARSI_BESLOUDNESS_H__


#include <BesLoudness_HD_exp.h>

#define BLOCK_SIZE 512
#define BLOCK_SIZE_LOW_LATENCY 256
#define DRC_NOISEFILTER_MIN 80 // means -80db/-75db
#define NOISE_FILTER_BASE -256
#define NOISE_FILTER_STEP 5
#define MIN_MULTI_BAND_COUNT 5
#define MAX_MULTI_BAND_COUNT 10
#define MUSIC_UP_BOUND 11   // mMusicCount > MUSIC_UP_BOUND, use MIN_MULTI_BAND_COUNT
#define MUSIC_LOW_BOUND 5   // mMusicCount < MUSIC_LOW_BOUND, use MAX_MULTI_BAND_COUNT


typedef enum
{
    AUDIO_COMP_FLT_AUDIO,               // ACF for Music/Ringtone Main Channel and DRC for Music only
    AUDIO_COMP_FLT_HEADPHONE,
    AUDIO_COMP_FLT_AUDENH,
    AUDIO_COMP_FLT_VIBSPK,
    AUDIO_COMP_FLT_AUDIO_SUB,           // ACF for Music/Ringtone Sub Channel
    AUDIO_COMP_FLT_DRC_FOR_MUSIC,
    AUDIO_COMP_FLT_DRC_FOR_RINGTONE,    // DRC for Ringtone Only
    AUDIO_COMP_FLT_HCF_FOR_USB,        // HCF for USB
    AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE,     // ACF for Ringtone
    AUDIO_COMP_FLT_NUM,
} AudioCompFltType_t;

typedef enum
{
    AUDIO_CMP_FLT_LOUDNESS_NONE = -1,       // NONE
    AUDIO_CMP_FLT_LOUDNESS_BASIC = 0,       // FILT_MODE_LOUD_FLT+LOUD_MODE_BASIC
    AUDIO_CMP_FLT_LOUDNESS_ENHANCED,        // FILT_MODE_LOUD_FLT+LOUD_MODE_ENHANCED
    AUDIO_CMP_FLT_LOUDNESS_AGGRESSIVE,      // FILT_MODE_LOUD_FLT+LOUD_MODE_AGGRESSIVE
    AUDIO_CMP_FLT_LOUDNESS_LITE,            // FILT_MODE_NONE+LOUD_MODE_BASIC
    AUDIO_CMP_FLT_LOUDNESS_COMP,            // FILT_MODE_COMP_FLT+LOUD_MODE_NONE
    AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC,      // FILT_MODE_COMP_FLT+LOUD_MODE_BASIC
    AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE,  // FILT_MODE_COMP_HDP+LOUD_MODE_NONE
    AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH,     // FILT_MODE_AUD_ENH+LOUD_MODE_NONE
    AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITHOUT_LIMITER, // HD_FILT_MODE_LOW_LATENCY_ACF+LOUD_MODE_NONE
    AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITH_LIMITER     // HD_FILT_MODE_LOW_LATENCY_ACF+LOUD_MODE_NONE
} AudioComFltMode_t;

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

typedef struct _AUDIO_ACF_V5F_PARAM
{
    unsigned int bes_loudness_L_hpf_fc;
    unsigned int bes_loudness_L_hpf_order;
    unsigned int bes_loudness_L_lpf_fc;
    unsigned int bes_loudness_L_lpf_order;
    unsigned int bes_loudness_L_bpf_fc[8];
    unsigned int bes_loudness_L_bpf_bw[8];
    int          bes_loudness_L_bpf_gain[8];
    unsigned int bes_loudness_R_hpf_fc;
    unsigned int bes_loudness_R_hpf_order;
    unsigned int bes_loudness_R_lpf_fc;
    unsigned int bes_loudness_R_lpf_order;
    unsigned int bes_loudness_R_bpf_fc[8];
    unsigned int bes_loudness_R_bpf_bw[8];
    int          bes_loudness_R_bpf_gain[8];
}AUDIO_ACF_V5F_PARAM;

typedef struct _AUDIO_ACF_V5VIBSPK_PARAM
{
    unsigned int bes_loudness_bpf_coeff[2][6][3];

}AUDIO_ACF_V5VIBSPK_PARAM;


typedef struct _AUDIO_ACF_CUSTOM_PARAM_STRUCT
{

    union{
        AUDIO_ACF_V5F_PARAM V5F;
        AUDIO_ACF_V5VIBSPK_PARAM V5ViVSPK;
    }bes_loudness_f_param;
    unsigned int bes_loudness_Sep_LR_Filter;
    unsigned int bes_loudness_WS_Gain_Max;
    unsigned int bes_loudness_WS_Gain_Min;
    unsigned int bes_loudness_Filter_First;
    unsigned int bes_loudness_Num_Bands;
    unsigned int bes_loudness_Flt_Bank_Order;
    unsigned int bes_loudness_Cross_Freq[7];
    int DRC_Th[8][5];
    int DRC_Gn[8][5];
    int SB_Gn[8];
    unsigned int SB_Mode[8];
    unsigned int DRC_Delay;
    unsigned int Att_Time[8][6];
    unsigned int Rel_Time[8][6];
    int Hyst_Th[8][6];
    int Lim_Th;
    int Lim_Gn;
    unsigned int Lim_Const;
    unsigned int Lim_Delay;
    int SWIPRev;
} AUDIO_ACF_CUSTOM_PARAM_STRUCT;

typedef enum {
    FILTER_BYPASS_ACF  = 0x1,
    FILTER_BYPASS_HCF  = 0x2,
    FILTER_BYPASS_DRC  = 0x4,
} FILTER_BYPASS_STATUS;

typedef enum {
    BLOUD_STATE_NONE,
    BLOUD_STATE_INIT,
    BLOUD_STATE_OPEN,
    BLOUD_STATE_PROCESS,
} BLOUD_STATUS;

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
    BLOUD_HD_InitParam mInitParam;
    BLOUD_HD_IIR_Design_Param mParamFormatUse;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT mCachedAudioParam;
    lib_workingbuf_info_t mLibWorkingBuf;
    audioloud_lib_param_t *pLibParamBuf;
    uint32_t mFilterType;
    uint32_t mStreamType;       // To Record the streamType before DRC off
    uint32_t mOutputDevice;
    uint32_t mScenario;
    uint32_t mBypassFlag;
    char mCustScene[20];
    bool bIsEnhaceOn;
    BLOUD_STATUS mState;
    uint32_t id;
} audioloud_lib_handle_t;






#endif // __ARSI_BESLOUDNESS_H__
