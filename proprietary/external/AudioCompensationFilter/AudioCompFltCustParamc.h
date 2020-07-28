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

/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioCompFltCustParamc.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements customized parameter handling in C
 *
 * Author:
 * -------
 *   HsinYi Chang (mtk09865)
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 *******************************************************************************/

#ifndef _AUDIO_COMP_FLT_CUST_PARAMC_H_
#define _AUDIO_COMP_FLT_CUST_PARAMC_H_

/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#include "CFG_AUDIO_File.h"


/*=============================================================================
 *                              Class definition
 *===========================================================================*/
typedef enum
{
    AUDIO_COMP_FLT_XML_NONE = 0,
    AUDIO_COMP_FLT_XML_AUDIO = 1,   //ACF for Music/Ringtone Main Channel and DRC for Music only
    AUDIO_COMP_FLT_XML_HEADPHONE = 2,
    AUDIO_COMP_FLT_XML_DRC_FOR_MUSIC = 4,
    AUDIO_COMP_FLT_XML_HCF_USB = 8,
} AudioCompFltXmlCallbackType_t;

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
    AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE, // ACF for Ringtone Only
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
// functions
/**
 * Get Audio Compensation Filter parameters from the storage
 */
int getAudioCompFltCustParamFromStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene);

/**
 * Update Audio Compensation Filter parameters into the storage
 */
int setAudioCompFltCustParamToStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam);

/**
 * Init the callback function of changing XML
 */
int getDRCControlOptionParamFromStorage(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara);

int audioComFltCustParamInit(void);

int getCallbackFilter(AudioCompFltType_t eFLTtype);

void resetCallbackFilter(void);

AudioCompFltXmlCallbackType_t mFilterType = AUDIO_COMP_FLT_XML_NONE;

#endif  //_AUDIO_COMP_FLT_CUST_PARAMC_H_




