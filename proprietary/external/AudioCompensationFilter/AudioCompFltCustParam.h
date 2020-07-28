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
 *   This is that set/get Audio Compensation Filter parameters into/from the storage
 */

#ifndef _AUDIO_COMP_FLT_CUST_PARAM_H_
#define _AUDIO_COMP_FLT_CUST_PARAM_H_
#include <utils/Log.h>
#include <utils/String8.h>
#include "CFG_AUDIO_File.h"

namespace android {

enum AudioCompFltType_t {
    AUDIO_COMP_FLT_AUDIO,               // ACF for Music/Ringtone Main Channel and DRC for Music only
    AUDIO_COMP_FLT_HEADPHONE,
    AUDIO_COMP_FLT_AUDENH,
    AUDIO_COMP_FLT_VIBSPK,
    AUDIO_COMP_FLT_AUDIO_SUB,           // ACF for Music/Ringtone Sub Channel
    AUDIO_COMP_FLT_DRC_FOR_MUSIC,
    AUDIO_COMP_FLT_DRC_FOR_RINGTONE,    // DRC for Ringtone Only
    AUDIO_COMP_FLT_NUM,
};

enum AudioComFltMode_t {
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
};

/**
 * Get Audio Compensation Filter parameters from the storage
 */
extern "C" int getAudioCompFltCustParamFromStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene = NULL);

/**
 * Update Audio Compensation Filter parameters into the storage
 */
extern "C" int setAudioCompFltCustParamToStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam);

/**
 * Init the callback function of changing XML
 */
int audioComFltCustParamInit(void);

/**
 * Get default parameters of Compensation Filter
 */
int getDefaultAudioCompFltParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam);

typedef int getFunAudioCompFltCustParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene);

typedef int setFunAudioCompFltCustParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam);


}; // namespace android

#endif  //_AUDIO_COMP_FLT_CUST_PARAM_H_
