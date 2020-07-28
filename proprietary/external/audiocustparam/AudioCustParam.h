/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioCustParam.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements custom parameter setting.
 *
 * Author:
 * -------
 *   HP Cheng (mtk01752)
 *
 *******************************************************************************/

#ifndef _AUDIO_CUST_PARAM_H_
#define _AUDIO_CUST_PARAM_H_


/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#include <utils/String8.h>
#include "CFG_AUDIO_File.h"


extern "C"
{

    typedef enum
    {
        SUPPORT_WB_SPEECH = 0x1,
        SUPPORT_DUAL_MIC = 0x2,
        SUPPORT_HD_RECORD = 0x4,
        SUPPORT_HD_48K_REC = 0x8,
        SUPPORT_DMNR_3_0 = 0x10,
        SUPPORT_DMNR_AT_MODEM = 0x20,
        SUPPORT_VOIP_ENHANCE = 0x40,
        SUPPORT_WIFI_ONLY = 0x80,
        SUPPORT_3G_DATA = 0x100,
        SUPPORT_NO_RECEIVER = 0x200,
        SUPPORT_ASR = 0x400,
        SUPPORT_VOIP_NORMAL_DMNR = 0x800,
        SUPPORT_VOIP_HANDSFREE_DMNR = 0x1000,
        SUPPORT_INCALL_NORMAL_DMNR = 0x2000,
        SUPPORT_INCALL_HANDSFREE_DMNR = 0x4000,
        SUPPORT_VOICE_UNLOCK = 0x8000,
        SUPPORT_DMNR_COMPLEX_ARCH = 0x10000,
        SUPPORT_GET_FO_VALUE = 0x20000,
        SUPPORT_SPEAKER_MONITOR = 0x40000,
        SUPPORT_BESLOUDNESS_V5 = 0x80000,
        SUPPORT_MAGI_CONFERENCE = 0x100000,
        SUPPORT_HAC = 0x200000,
        SUPPORT_SPEECH_LPBK = 0x400000,
        SUPPORT_BT_GAIN_TABLE = 0x800000,
        SUPPORT_AUDIO_BT_NREC_WO_ENH = 0x1000000,
        SUPPORT_AUDIO_SKIP_FM_MATV_VOL_TUNING = 0x2000000,
        SUPPORT_HFP = 0x4000000,
        SUPPORT_AUDIO_LAYERED_PARAM = 0x8000000,
        PHASE_OUT_HEADPHONE_COMPENSATION_FILTER = 0x10000000,
        FEATURE_SUPPORT_INFO_LIST_END
    } FeatureSupportInfo;


    /*=============================================================================
     *                              Class definition
     *===========================================================================*/

    int getNumMicSupport(void);

    uint32_t   QueryFeatureSupportInfo(void);

    // NB speech parameters
    // NB speech parameters
    int            GetNBSpeechParamFromNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
    int            SetNBSpeechParamToNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);

    // Dual mic speech parameters
    int            GetDualMicSpeechParamFromNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);
    int            SetDualMicSpeechParamToNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);

    // WB speech parameters
    int            GetWBSpeechParamFromNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);
    int            SetWBSpeechParamToNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);

    // Med param parameter
    int            GetMedParamFromNV(AUDIO_PARAM_MED_STRUCT *pPara);
    int            SetMedParamToNV(AUDIO_PARAM_MED_STRUCT *pPara);

    // VolumeVer1 parameter
    int            GetVolumeVer1ParamFromNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara);
    int            SetVolumeVer1ParamToNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara);

    // Audio Custom Paramete
    int            GetAudioCustomParamFromNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara);
    int            SetAudioCustomParamToNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara);

    // AudioGainTable Parameter
    int            GetAudioGainTableParamFromNV(AUDIO_GAIN_TABLE_STRUCT *pPara);
    int            SetAudioGainTableParamToNV(AUDIO_GAIN_TABLE_STRUCT *pPara);

    // Audio HD record parameters
    int            GetHdRecordParamFromNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara);
    int            SetHdRecordParamToNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara);

    //  Audio HD record scene table
    int            GetHdRecordSceneTableFromNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara);
    int            SetHdRecordSceneTableToNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara);

    // Audio VoIP Parameters
    int            GetAudioVoIPParamFromNV(AUDIO_VOIP_PARAM_STRUCT *pPara);
    int            SetAudioVoIPParamToNV(AUDIO_VOIP_PARAM_STRUCT *pPara);

    // Audio HFP Parameters
    int            GetAudioHFPParamFromNV(AUDIO_HFP_PARAM_STRUCT *pPara);
    int            SetAudioHFPParamToNV(AUDIO_HFP_PARAM_STRUCT *pPara);

    // MagiConference Call parameters
    int            GetMagiConSpeechParamFromNVRam(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon);
    int            SetMagiConSpeechParamToNVRam(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon);

    // HAC parameters
    int            GetHACSpeechParamFromNVRam(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC);
    int            SetHACSpeechParamToNVRam(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC);

    // Speech Loopback parameters
    int            GetNBSpeechLpbkParamFromNVRam(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk);
    int            SetNBSpeechLpbkParamToNVRam(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk);

    // BT Gain parameter
    int            GetAudioBTGainParamFromNV(AUDIO_BT_GAIN_STRUCT *pParaBT);
    int            SetAudioBTGainParamToNV(AUDIO_BT_GAIN_STRUCT *pParaBT);

}

#endif




