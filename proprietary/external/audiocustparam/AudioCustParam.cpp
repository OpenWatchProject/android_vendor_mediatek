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
 * AudioCustParam.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements customized parameter handling
 *
 * Author:
 * -------
 *   HP Cheng (mtk01752)
 *
 *------------------------------------------------------------------------------
 * $Revision: #2 $
 * $Modtime:$
 * $Log:$
 *
 * 06 05 2013 donglei.ji
 * [ALPS00683353] [Need Patch] [Volunteer Patch] DMNR3.0 and VOIP tuning check in
 * .
 *
 * 12 29 2012 donglei.ji
 * [ALPS00425279] [Need Patch] [Volunteer Patch] voice ui and password unlock feature check in
 * voice ui - NVRAM .
 *
 *
 *******************************************************************************/

/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioCustParam"
#include <utils/Log.h>

#include "AudioCustParam.h"
#include <cutils/properties.h>
#include <utils/String8.h>

#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
#include "AudioParamParser.h"
#endif

#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "unistd.h"
#include "pthread.h"
#endif

#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "CFG_Audio_Default.h"
#ifndef MTK_BASIC_PACKAGE
#include "audio_custom_exp.h"
#endif

#ifdef MTK_BASIC_PACKAGE
#define USE_DEFAULT_CUST_TABLE //For BringUp usage
#endif

#define MAX_RETRY_COUNT 30

extern "C"
{

    /*=============================================================================
     *                             Public Function
     *===========================================================================*/

    bool checkNvramReady(void) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return true;
#endif
        int read_nvram_ready_retry = 0;
        int ret = 0;
        char nvram_init_val[PROPERTY_VALUE_MAX];
        while (read_nvram_ready_retry < MAX_RETRY_COUNT) {
            read_nvram_ready_retry++;
            property_get("vendor.service.nvram_init", nvram_init_val, NULL);
            if (strcmp(nvram_init_val, "Ready") == 0 ||
                strcmp(nvram_init_val, "Pre_Ready") == 0) {
                ret = true;
                break;
            } else {
                ALOGD("%s(), property_get(\"vendor.service.nvram_init\") = %s, read_nvram_ready_retry = %d",
                      __FUNCTION__, nvram_init_val, read_nvram_ready_retry);
                usleep(500 * 1000);
            }
        }
        if (read_nvram_ready_retry >= MAX_RETRY_COUNT) {
            ALOGW("Get nvram restore ready faild !!!\n");
            ret = false;
        }
        return ret;
    }

    int getNumMicSupport() {
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
        /* Get AudioParamOptions.xml Project config */
        AppOps *appOps = appOpsGetInstance();
        if (appOps == NULL) {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
#if defined(MTK_DUAL_MIC_SUPPORT)
            return 2;
#else
            return 1;
#endif
        }

        AppHandle *appHandle = appOps->appHandleGetInstance();

        int numMic = 2;
        int dualMicFlag = appOps->appHandleIsFeatureOptionEnabled(appHandle, "MTK_DUAL_MIC_SUPPORT");
        const char *numMicString = appOps->appHandleGetFeatureOptionValue(appHandle, "MTK_AUDIO_NUMBER_OF_MIC");

#if defined(CONFIG_MT_ENG_BUILD)
        ALOGD("%s(), dualMicFlag %d, numMicString %s", __FUNCTION__, dualMicFlag, numMicString);
#endif
        if (numMicString == NULL) {
            numMic = (dualMicFlag == 1) ? 2 : 1;
            ALOGW("%s(), MTK_AUDIO_NUMBER_OF_MIC undefined in AudioParamOptions, set numMic = %d",
              __FUNCTION__, numMic);
        } else {
            numMic = atoi(numMicString);
            if (dualMicFlag == 1) { // priority MTK_DUAL_MIC_SUPPORT > MTK_AUDIO_NUMBER_OF_MIC
                if (numMic < 2) {
                    ALOGW("%s(), numMic %d < 2, confict with dualMicFlag, set numMic == 2",
                          __FUNCTION__, numMic);
                    numMic = 2;
                }
            } else {
                numMic = 1;
            }
        }
        return numMic;
#else
#if defined(MTK_DUAL_MIC_SUPPORT)
        return 2;
#else
        return 1;
#endif
#endif
    }

    uint32_t QueryFeatureSupportInfo() {
        uint32_t RetInfo = 0;
        bool bForceEnable = false;
        bool bDUAL_MIC_SUPPORT = false;
        bool bVOIP_ENHANCEMENT_SUPPORT = false;
        bool bASR_SUPPORT = false;
        bool bVOIP_NORMAL_DMNR_SUPPORT = false;
        bool bVOIP_HANDSFREE_DMNR_SUPPORT = false;
        bool bINCALL_NORMAL_DMNR_SUPPORT = false;
        bool bINCALL_HANDSFREE_DMNR_SUPPORT = false;
        bool bNoReceiver = false;
        bool bNoSpeech = false;
        bool bWifiOnly = false;
        bool b3GDATAOnly = false;
        bool bWideBand = false;
        bool bHDRecord = false;
        bool bDMNR_3_0 = false;
        bool bDMNRTuningAtModem = false;
        bool bVoiceUnlock = false;
        bool bDMNR_COMPLEX_ARCH_SUPPORT = false;
        bool bGET_FO = false;
        bool bSupportBesloudnessV5 = false;
        bool bMagiConference = false;
        bool bHAC = false;
        bool bLPBK = false;
        bool bBTGain = false;
        bool bBTNREC = false;
        bool bSkipFmMatvVolumeTuning = true; //Ture in L
        bool bHFP = false;
        bool bAudioLayeredParam = false;
        bool bPhaseOutHCF = false;

        if (getNumMicSupport() >= 2) {
            bDUAL_MIC_SUPPORT = true;
        }

#ifdef MTK_VOIP_ENHANCEMENT_SUPPORT
        bVOIP_ENHANCEMENT_SUPPORT = true;
#endif

#ifdef MTK_ASR_SUPPORT
        bASR_SUPPORT = true;
#endif

#ifdef MTK_VOIP_NORMAL_DMNR
        bVOIP_NORMAL_DMNR_SUPPORT = true;
#endif

#ifdef MTK_VOIP_HANDSFREE_DMNR
        bVOIP_HANDSFREE_DMNR_SUPPORT = true;
#endif

#ifdef MTK_INCALL_HANDSFREE_DMNR
        bINCALL_HANDSFREE_DMNR_SUPPORT = true;
#endif

#ifdef MTK_INCALL_NORMAL_DMNR
        bINCALL_NORMAL_DMNR_SUPPORT = true;
#endif

#ifdef MTK_DISABLE_EARPIECE  // DISABLE_EARPIECE
        bNoReceiver = true;
#endif

#ifdef MTK_WIFI_ONLY_SUPPORT
        bWifiOnly = true;
#endif

#ifdef MTK_3G_DATA_ONLY_SUPPORT
        b3GDATAOnly = true;
#endif

#ifdef MTK_WB_SPEECH_SUPPORT
        bWideBand = true;
#endif

#ifdef MTK_AUDIO_HD_REC_SUPPORT
        bHDRecord = true;
#endif

#ifdef MTK_HANDSFREE_DMNR_SUPPORT
        bDMNR_3_0 = true;
#endif

#ifdef DMNR_TUNNING_AT_MODEMSIDE
        bDMNRTuningAtModem = true;
#endif

#if defined(MTK_VOICE_UNLOCK_SUPPORT) || defined(MTK_VOICE_UI_SUPPORT)
        bVoiceUnlock = true;
#endif

#ifdef DMNR_COMPLEX_ARCH_SUPPORT
        bDMNR_COMPLEX_ARCH_SUPPORT = true;
#endif

#ifdef MTK_ACF_AUTO_GEN_SUPPORT
        bGET_FO = true;
#endif

        bSupportBesloudnessV5 = true;

#if defined(MTK_MAGICONFERENCE_SUPPORT)
        if (getNumMicSupport() >= 2) {
            bMagiConference = true;
        }
#endif

#if defined(MTK_HAC_SUPPORT)
        bHAC = true;
#endif

#if defined(MTK_AUDIO_SPH_LPBK_PARAM)
        bLPBK = true;
#endif
#if defined(MTK_AUDIO_GAIN_TABLE_BT)
        bBTGain = true;
#endif
#if defined(MTK_AUDIO_BT_NREC_WO_ENH_MODE)
        bBTNREC = true;
#endif
#ifdef MTK_WEARABLE_PLATFORM
        bHFP = true;
#endif
#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
        bAudioLayeredParam = true;
#endif
#ifndef ENABLE_HEADPHONE_COMPENSATION_FILTER
        bPhaseOutHCF = true;
#endif
        if (bWifiOnly || b3GDATAOnly) {
            bNoSpeech = true;
        }

        bVOIP_ENHANCEMENT_SUPPORT = true;
        if (bDUAL_MIC_SUPPORT) {
            bForceEnable = true;
        }

        // SUPPORT_WB_SPEECH
        if (bWideBand) {
            RetInfo = RetInfo | SUPPORT_WB_SPEECH;
        }

        // SUPPORT_DUAL_MIC
        if (bDUAL_MIC_SUPPORT) {
            RetInfo = RetInfo | SUPPORT_DUAL_MIC;
        }

        // SUPPORT_HD_RECORD
        if (bHDRecord) {
            RetInfo = RetInfo | SUPPORT_HD_RECORD;
        }

        // SUPPORT_DMNR_3_0
        if (bDMNR_3_0) {
            RetInfo = RetInfo | SUPPORT_DMNR_3_0;
        }

        //SUPPORT_DMNR_AT_MODEM
        if (bDMNRTuningAtModem) {
            RetInfo = RetInfo | SUPPORT_DMNR_AT_MODEM;
        }

        //SUPPORT_VOIP_ENHANCE
        if (bVOIP_ENHANCEMENT_SUPPORT) {
            RetInfo = RetInfo | SUPPORT_VOIP_ENHANCE;
        }

        //SUPPORT_WIFI_ONLY
        if (bWifiOnly) {
            RetInfo = RetInfo | SUPPORT_WIFI_ONLY;
        }

        //SUPPORT_3G_DATA
        if (b3GDATAOnly) {
            RetInfo = RetInfo | SUPPORT_3G_DATA;
        }

        //SUPPORT_NO_RECEIVER
        if (bNoReceiver) {
            RetInfo = RetInfo | SUPPORT_NO_RECEIVER;
        }

        //SUPPORT_ASR
        if (bDUAL_MIC_SUPPORT && (bASR_SUPPORT || bForceEnable)) {
            RetInfo = RetInfo | SUPPORT_ASR;
        }

        //SUPPORT_VOIP_NORMAL_DMNR
        if (!bNoReceiver) {
            if (bDUAL_MIC_SUPPORT && ((bVOIP_NORMAL_DMNR_SUPPORT && bVOIP_ENHANCEMENT_SUPPORT) || bForceEnable)) {
                RetInfo = RetInfo | SUPPORT_VOIP_NORMAL_DMNR;
            }
        }

        //SUPPORT_VOIP_HANDSFREE_DMNR
        if (bDUAL_MIC_SUPPORT && ((bVOIP_HANDSFREE_DMNR_SUPPORT && bVOIP_ENHANCEMENT_SUPPORT) || bForceEnable)) {
            RetInfo = RetInfo | SUPPORT_VOIP_HANDSFREE_DMNR;
        }

        //SUPPORT_INCALL_NORMAL_DMNR
        if (!bNoReceiver && !bNoSpeech) {
            if (bDUAL_MIC_SUPPORT && (bINCALL_NORMAL_DMNR_SUPPORT || bForceEnable)) {
                RetInfo = RetInfo | SUPPORT_INCALL_NORMAL_DMNR;
            }
        }

        //SUPPORT_INCALL_HANDSFREE_DMNR
        if (!bNoSpeech) {
            if (bDUAL_MIC_SUPPORT && (bINCALL_HANDSFREE_DMNR_SUPPORT || bForceEnable)) {
                RetInfo = RetInfo | SUPPORT_INCALL_HANDSFREE_DMNR;
            }
        }

        //SUPPORT_VOICE_UNLOCK
        if (bVoiceUnlock) {
            RetInfo = RetInfo | SUPPORT_VOICE_UNLOCK;
        }

        //DMNR_COMPLEX_ARCH_SUPPORT
        if (bDMNR_COMPLEX_ARCH_SUPPORT) {
            RetInfo = RetInfo | SUPPORT_DMNR_COMPLEX_ARCH;
        }

        if (bGET_FO) {
            RetInfo = RetInfo | SUPPORT_GET_FO_VALUE;
        }

        if (bSupportBesloudnessV5) {
            RetInfo = RetInfo | SUPPORT_BESLOUDNESS_V5;
        }
        if (bMagiConference) {
            RetInfo = RetInfo | SUPPORT_MAGI_CONFERENCE;
        }
        if (bHAC) {
            RetInfo = RetInfo | SUPPORT_HAC;
        }
        if (bLPBK) {
            RetInfo = RetInfo | SUPPORT_SPEECH_LPBK;
        }
        if (bBTGain) {
            RetInfo = RetInfo | SUPPORT_BT_GAIN_TABLE;
        }

        if (bBTNREC) {
            RetInfo = RetInfo | SUPPORT_AUDIO_BT_NREC_WO_ENH;
        }

        if (bSkipFmMatvVolumeTuning) {
            RetInfo = RetInfo | SUPPORT_AUDIO_SKIP_FM_MATV_VOL_TUNING;
        }
        if (bHFP) {
            RetInfo = RetInfo | SUPPORT_HFP;
        }

        if (bAudioLayeredParam) {
            RetInfo = RetInfo | SUPPORT_AUDIO_LAYERED_PARAM;
        }

        if (bPhaseOutHCF) {
            RetInfo = RetInfo | PHASE_OUT_HEADPHONE_COMPENSATION_FILTER;
        }

#if defined(CONFIG_MT_ENG_BUILD)
        ALOGD("%s(), feature support bit 0x%x ", __FUNCTION__, RetInfo);
#endif
        return RetInfo;
    }

    int getDefaultSpeechParam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB) {
        // only for startup use
        ALOGW("Digi_DL_Speech = %u", speech_custom_default.Digi_DL_Speech);
        ALOGW("uMicbiasVolt = %u", speech_custom_default.uMicbiasVolt);
        ALOGW("sizeof AUDIO_CUSTOM_PARAM_STRUCT = %zu", sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        if (!pSphParamNB) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)pSphParamNB, (void *)&speech_custom_default, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        return sizeof(AUDIO_CUSTOM_PARAM_STRUCT);
    }

    int getDefaultVer1VolumeParam(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *volume_param) {
        // only for startup use
        ALOGD("getDefaultVer1VolumeParam");
        if (!volume_param) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)volume_param, (void *) & (audio_ver1_custom_default), sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT));
        return sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT);
    }

    // functions
    int GetVolumeVer1ParamFromNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara) {
        ALOGD("GetVolumeVer1ParamFromNV ");
        int result = 0;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return getDefaultVer1VolumeParam(pPara);
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }

#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultVer1VolumeParam(pPara);
        // get from NV ram and replace the default value
#else
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_VER1_VOLUME_CUSTOM_LID;
        int rec_size, rec_num;

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("GetVolumeVer1ParamFromNV rec_size = %d rec_num = %d", rec_size, rec_num);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT)) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT));
            result = getDefaultVer1VolumeParam(pPara);
        }
#endif
        return result;

    }

    int SetVolumeVer1ParamToNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_VER1_VOLUME_CUSTOM_LID;
        int rec_size, rec_num, result;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("SetVolumeVer1ParamToNV audio_nvram_fd.lid = %d", audio_nvram_fd.ifile_lid);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);
        return result;
    }


    int GetNBSpeechParamFromNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB) {
        int result = 0;
        if (checkNvramReady() == false) {
            ALOGE("checkNvramReady fail");
            return 0;
        } else if (!pSphParamNB) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }

#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value, should disable when NVRAM ready
        result = getDefaultSpeechParam(pSphParamNB);
        // get from NVRam and replace the default value
#else
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_LID;
        int rec_size, rec_num;

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("GetNBSpeechParamFromNVRam audio_nvram_fd.lid = %d, rec_size = %d, rec_num = %d, uMicbiasVolt = %d",
              audio_nvram_fd.ifile_lid, rec_size, rec_num, pSphParamNB->uMicbiasVolt);
        result = read(audio_nvram_fd.iFileDesc, pSphParamNB , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
            result = getDefaultSpeechParam(pSphParamNB);
        }
#endif

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
        memset(pSphParamNB, 0xff, result);
        ALOGW("%s(), Only support XML access, Reset the struct value with 0xff, size = %d\n", __FUNCTION__, result);
#endif

        return result;
    }

    int SetNBSpeechParamToNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_LID;
        int rec_size, rec_num, result;
        if (!pSphParamNB) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("SetNBSpeechParamToNVRam audio_nvram_fd.lid = %d", audio_nvram_fd.ifile_lid);
        result = write(audio_nvram_fd.iFileDesc, pSphParamNB , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        return result;
    }

    int getDefaultAudioGainTableParam(AUDIO_GAIN_TABLE_STRUCT *sphParam) {
        ALOGW("sizeof AUDIO_GAIN_TABLE_STRUCT = %zu", sizeof(AUDIO_GAIN_TABLE_STRUCT));
        if (!sphParam) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)sphParam, (void *) & (Gain_control_table_default), sizeof(AUDIO_GAIN_TABLE_STRUCT));
        return sizeof(AUDIO_GAIN_TABLE_STRUCT);
    }

    int GetAudioGainTableParamFromNV(AUDIO_GAIN_TABLE_STRUCT *pPara) {
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID;
        int rec_size, rec_num, result;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        result = getDefaultAudioGainTableParam(pPara);
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("GetAudioGainTableParamFromNV audio_nvram_fd.lid = %d rec_size = %d rec_num = %d", audio_nvram_fd.ifile_lid, rec_size, rec_num);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_GAIN_TABLE_STRUCT));
            result = getDefaultAudioGainTableParam(pPara);
        }
#endif
        return result;
    }

    int SetAudioGainTableParamToNV(AUDIO_GAIN_TABLE_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID;
        int rec_size, rec_num, result;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("SetAudioGainTableParamToNV audio_nvram_fd.lid = %d", audio_nvram_fd.ifile_lid);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        return result;
    }


    int getDefaultWBSpeechParam(AUDIO_CUSTOM_WB_PARAM_STRUCT *sphParam) {
        ALOGW("sizeof AUDIO_CUSTOM_WB_PARAM_STRUCT = %zu", sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        if (!sphParam) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)sphParam, (void *) & (wb_speech_custom_default), sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        return sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT);
    }


    int GetWBSpeechParamFromNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB) {
        int result = 0;
        // a default value , should disable when NVRAM ready
        //getDefaultWBSpeechParam(pSphParamWB);
        // get from NV ram and replace the default value


        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pSphParamWB) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }

#if defined(USE_DEFAULT_CUST_TABLE)
        result = getDefaultWBSpeechParam(pSphParamWB);
#else

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("GetWBSpeechParamFromNVRam audio_nvram_fd.lid = %d, rec_size = %d, rec_num = %d",
              audio_nvram_fd.ifile_lid, rec_size, rec_num);
        result = read(audio_nvram_fd.iFileDesc, pSphParamWB , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
            result = getDefaultWBSpeechParam(pSphParamWB);
        }
#endif

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
        memset(pSphParamWB, 0xff, result);
        ALOGW("%s(), Only support XML access, Reset the struct value with 0xff\n", __FUNCTION__);
#endif

        return result;
    }

    int SetWBSpeechParamToNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID;
        int rec_size, rec_num, result;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("SetWBSpeechParamToNVRam audio_nvram_fd.lid = %d", audio_nvram_fd.ifile_lid);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        return result;
    }

    int getDefaultMedParam(AUDIO_PARAM_MED_STRUCT *pPara) {
        // only for startup use
        ALOGW("sizeof AUDIO_PARAM_MED_STRUCT = %zu", sizeof(AUDIO_PARAM_MED_STRUCT));
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)pPara, (void *) & (audio_param_med_default), sizeof(AUDIO_PARAM_MED_STRUCT));
        return sizeof(AUDIO_PARAM_MED_STRUCT);
    }

    int GetMedParamFromNV(AUDIO_PARAM_MED_STRUCT *pPara) {
        int result = 0;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }

#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultMedParam(pPara);
        // get from NV ram and replace the default value
#else
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID;
        int rec_size, rec_num;

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_PARAM_MED_STRUCT));
            result = getDefaultMedParam(pPara);
        }
#endif
        return result;
    }

    int SetMedParamToNV(AUDIO_PARAM_MED_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        // write to NV ram
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID;
        int rec_size, rec_num, result;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        return result;
    }

    int getDefaultAudioCustomParam(AUDIO_VOLUME_CUSTOM_STRUCT *volParam) {
        // only for startup use
        ALOGW("sizeof AUDIO_VOLUME_CUSTOM_STRUCT = %zu", sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
        if (!volParam) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)volParam, (void *) & (audio_volume_custom_default), sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
        return sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
    }

    // get audio custom parameter from NVRAM
    int GetAudioCustomParamFromNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara) {
        int result = 0;
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultAudioCustomParam(pPara);
        // get from NV ram and replace the default value
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != sizeof(AUDIO_VOLUME_CUSTOM_STRUCT)) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
            result = getDefaultAudioCustomParam(pPara);
        }
#endif
        return result;
    }

    int SetAudioCustomParamToNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID;
        int rec_size, rec_num, result;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        return result;
    }



    //////////////////////////////////////////////
    // Dual Mic Custom Parameter
    //////////////////////////////////////////////

    int getDefaultDualMicParam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *dualMicParam) {
        ALOGD("sizeof AUDIO_CUSTOM_PARAM_STRUCT = %zu", sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        if (!dualMicParam) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)dualMicParam, (void *) & (dual_mic_custom_default), sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        return sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT);
    }

    // Get Dual Mic Custom Parameter from NVRAM
    int GetDualMicSpeechParamFromNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic) {
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pSphParamDualMic) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }

        F_ID dualmic_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID;
        int rec_size, rec_num, result;
#if defined(USE_DEFAULT_CUST_TABLE)
        // for test only
        // Get the Dual Mic default parameter, (Disable it when NVRAM ready)
        result = getDefaultDualMicParam(pSphParamDualMic);
        // get from NV ram and replace the default value
#else
        result = 0;
        dualmic_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("+GetDualMicSpeechParamFromNVRam audio_nvram_fd.lid = %d, rec_size=%d, rec_num=%d", dualmic_nvram_fd.ifile_lid, rec_size, rec_num);
        result = read(dualmic_nvram_fd.iFileDesc, pSphParamDualMic , rec_size * rec_num);
        NVM_CloseFileDesc(dualmic_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
            result = getDefaultDualMicParam(pSphParamDualMic);
        }
#endif
        return result;
    }

    // Set Dual Mic Custom Parameter from NVRAM
    int SetDualMicSpeechParamToNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        F_ID dualmic_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID;
        int rec_size, rec_num, result;
        result = 0;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        dualmic_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("+SetDualMicSpeechParamToNVRam audio_nvram_fd.lid = %d", dualmic_nvram_fd.ifile_lid);
        result = write(dualmic_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(dualmic_nvram_fd);
        return result;
    }

    //////////////////////////////////////////////
    // HD Record Custom Parameter
    //////////////////////////////////////////////
    int getDefaultHdRecordParam(AUDIO_HD_RECORD_PARAM_STRUCT *pPara) {
        ALOGD("sizeof AUDIO_HD_RECORD_PARAM_STRUCT = %zu", sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)pPara, (void *) & (Hd_Recrod_Par_default), sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
        return sizeof(AUDIO_HD_RECORD_PARAM_STRUCT);
    }

    /// Get HD record parameters from NVRAM
    int GetHdRecordParamFromNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;
#if defined(MTK_AUDIO_HD_REC_SUPPORT)

        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultHdRecordParam(pPara);
        // get from NV ram and replace the default value
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("GetHdRecordParamFromNV rec_size = %d rec_num = %d", rec_size, rec_num);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);
        ALOGD("GetHdRecordParamFromNV result = %d", result);
        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
            result = getDefaultHdRecordParam(pPara);
        }
#endif
#else
        ALOGW("-%s(), HdRecord not support!", __FUNCTION__);
#endif
        return result;
    }

    /// Set HD record parameters to NVRAM
    int SetHdRecordParamToNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        result = 0;
#else
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID;
        int rec_size, rec_num;
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("SetHdRecordParamToNV audio_nvram_fd.lid = %d rec_size = %d rec_num = %d", audio_nvram_fd.ifile_lid, rec_size, rec_num);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);
#endif
#else
        ALOGW("-%s(), HdRecord not support!", __FUNCTION__);
#endif
        return result;
    }
    int getDefaultHdRecordSceneTable(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara) {
        ALOGD("sizeof AUDIO_HD_RECORD_SCENE_TABLE_STRUCT = %zu", sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        memcpy((void *)pPara, (void *) & (Hd_Recrod_Scene_Table_default), sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
        return sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT);
    }
    /// Get HD record scene tables from NVRAM
    int GetHdRecordSceneTableFromNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }

#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultHdRecordSceneTable(pPara);
        // get from NV ram and replace the default value
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("GetHdRecordSceneTableFromNV rec_size = %d rec_num = %d", rec_size, rec_num);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);
        ALOGD("GetHdRecordSceneTableFromNV result = %d", result);
        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
            result = getDefaultHdRecordSceneTable(pPara);
        }
#endif
#else
        ALOGW("-%s(), HdRecord not support!", __FUNCTION__);
#endif
        return result;
    }

    /// Set HD record scene tables to NVRAM
    int SetHdRecordSceneTableToNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        result = 0;
#else
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID;
        int rec_size, rec_num;
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("SetHdRecordSceneTableToNV audio_nvram_fd.lid = %d rec_size = %d rec_num = %d", audio_nvram_fd.ifile_lid, rec_size, rec_num);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);
#endif
#else
        ALOGW("-%s(), HdRecord not support!", __FUNCTION__);
#endif

        return result;
    }

    int getDefaultAudioVoIPParam(AUDIO_VOIP_PARAM_STRUCT *pPara) {
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        ALOGD("sizeof AUDIO_VOIP_PARAM_STRUCT = %zu", sizeof(AUDIO_VOIP_PARAM_STRUCT));
        memcpy((void *)pPara, (void *) & (Audio_VOIP_Par_default), sizeof(AUDIO_VOIP_PARAM_STRUCT));
        return sizeof(AUDIO_VOIP_PARAM_STRUCT);
    }


    /// Get VoIP parameters from NVRAM
    int GetAudioVoIPParamFromNV(AUDIO_VOIP_PARAM_STRUCT *pPara) {
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID;
        int rec_size, rec_num, result;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        result = getDefaultAudioVoIPParam(pPara);
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("GetAudioVoIPParamFromNV rec_size = %d rec_num = %d", rec_size, rec_num);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        ALOGD("GetAudioVoIPParamFromNV result = %d", result);
        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_VOIP_PARAM_STRUCT));
            result = getDefaultAudioVoIPParam(pPara);
        }
#endif
        return result;
    }

    /// Set VoIP parameters to NVRAM
    int SetAudioVoIPParamToNV(AUDIO_VOIP_PARAM_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID;
        int rec_size, rec_num, result;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID audio_nvram_fd.lid = %d rec_size = %d rec_num = %d", audio_nvram_fd.ifile_lid, rec_size, rec_num);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        return result;
    }

    int getDefaultMagiConSpeechParam(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon) {
        if (!pSphParamMagiCon) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        // only for startup use
        ALOGW("sizeof AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT = %zu", sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
        memcpy((void *)pSphParamMagiCon, (void *)&speech_magi_conference_custom_default, sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
        return sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT);
    }

    // Get Magic Conference Call parameters from NVRAM
    int GetMagiConSpeechParamFromNVRam(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;

        if (getNumMicSupport() < 2) {
            ALOGW("-%s(), num mic < 2, MagiConference not support!", __FUNCTION__);
            return result;
        }

#if defined(MTK_MAGICONFERENCE_SUPPORT)
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_MAGI_CONFERENCE_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            (void)pSphParamMagiCon;
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pSphParamMagiCon) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultMagiConSpeechParam(pSphParamMagiCon);
        // get from NV ram and replace the default value
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        result = read(audio_nvram_fd.iFileDesc, pSphParamMagiCon , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
            result = getDefaultMagiConSpeechParam(pSphParamMagiCon);
        }
#endif
#else
        (void)pSphParamMagiCon;
        ALOGW("-%s(), MagiConference not support!", __FUNCTION__);
#endif
        return result;
    }

    // Set Magic Conference Call parameters to NVRAM
    int SetMagiConSpeechParamToNVRam(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;

        if (getNumMicSupport() < 2) {
            ALOGW("-%s(), num mic < 2, MagiConference not support!", __FUNCTION__);
            return result;
        }

#if defined(MTK_MAGICONFERENCE_SUPPORT)
        if (!pSphParamMagiCon) {
            (void)pSphParamMagiCon;
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        result = 0;
#else

        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_MAGI_CONFERENCE_LID;
        int rec_size, rec_num;

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        result = write(audio_nvram_fd.iFileDesc, pSphParamMagiCon , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

#endif
#else
        (void)pSphParamMagiCon;
        ALOGW("-%s(), MagiConference not support!", __FUNCTION__);
#endif
        return result;
    }

    int getDefaultSpeechHacParam(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC) {
        if (!pSphParamHAC) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        // only for startup use
        ALOGW("sizeof AUDIO_CUSTOM_HAC_PARAM_STRUCT = %zu", sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
        memcpy((void *)pSphParamHAC, (void *)&speech_hac_param_custom_default, sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
        return sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT);
    }

    // Get HAC parameters from NVRAM
    int GetHACSpeechParamFromNVRam(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;
#if defined(MTK_HAC_SUPPORT)
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HAC_PARAM_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pSphParamHAC) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultSpeechHacParam(pSphParamHAC);
        // get from NV ram and replace the default value
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        result = read(audio_nvram_fd.iFileDesc, pSphParamHAC , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters, result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
            result = getDefaultSpeechHacParam(pSphParamHAC);
        }
#endif
#else
        (void)pSphParamHAC;
        ALOGW("-%s(), HAC not support!", __FUNCTION__);
#endif
        return result;
    }

    // Set HAC parameters to NVRAM
    int SetHACSpeechParamToNVRam(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC) {
        ALOGD("%s()", __FUNCTION__);
        int result = 0;
#if defined(MTK_HAC_SUPPORT)
#if defined(USE_DEFAULT_CUST_TABLE)
        (void)pSphParamHAC;
        result = 0;
#else
        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HAC_PARAM_LID;
        int rec_size, rec_num;
        if (!pSphParamHAC) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        result = write(audio_nvram_fd.iFileDesc, pSphParamHAC , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

#endif
#else
        (void)pSphParamHAC;
        ALOGW("-%s(), HAC not support!", __FUNCTION__);
#endif
        return result;
    }

    int getDefaultAudioHFPParam(AUDIO_HFP_PARAM_STRUCT *pPara) {
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        ALOGD("sizeof AUDIO_HFP_PARAM_STRUCT = %zu", sizeof(AUDIO_HFP_PARAM_STRUCT));
        memcpy((void *)pPara, (void *) & (audio_hfp_param_custom_default), sizeof(AUDIO_HFP_PARAM_STRUCT));
        return sizeof(AUDIO_HFP_PARAM_STRUCT);
    }


    /// Get HFP parameters from NVRAM
    int GetAudioHFPParamFromNV(AUDIO_HFP_PARAM_STRUCT *pPara) {
        int result = 0;
#ifdef MTK_WEARABLE_PLATFORM
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HFP_PAR_LID;
        if (!checkNvramReady()) {
            (void)pPara;
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }

#if defined(USE_DEFAULT_CUST_TABLE)
        result = getDefaultAudioHFPParam(pPara);
#else
        F_ID audio_nvram_fd ;
        int rec_size = 0, rec_num = 0;
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        ALOGD("%s  rec_size = %d rec_num = %d", __func__, rec_size, rec_num);
        result = read(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);
        ALOGD("%s result = %d", __func__, result);
        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_HFP_PARAM_STRUCT));
            result = getDefaultAudioHFPParam(pPara);
        }
#endif
#else
        (void)pPara;
#endif
        return result;
    }

    /// Set HFP parameters to NVRAM
    int SetAudioHFPParamToNV(AUDIO_HFP_PARAM_STRUCT *pPara) {
#if defined(USE_DEFAULT_CUST_TABLE)
        return 0;
#endif
        int result = 0;
        if (!pPara) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#ifdef MTK_WEARABLE_PLATFORM
        F_ID audio_nvram_fd ;
        int rec_size = 0, rec_num = 0;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_HFP_PAR_LID;

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        ALOGD("SetAudioHFPParamToNV audio_nvram_fd.lid = %d rec_size = %d rec_num = %d", audio_nvram_fd.ifile_lid, rec_size, rec_num);
        result = write(audio_nvram_fd.iFileDesc, pPara , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);
#endif
        return result;
    }

    int getDefaultSpeechNBLpbkParam(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk) {
        if (!pSphParamNBLpbk) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
        // only for startup use
        ALOGW("sizeof AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT = %zu", sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));
        memcpy((void *)pSphParamNBLpbk, (void *)&speech_lpbk_param_custom_default, sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));
        return sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT);
    }

    // Speech Loopback parameters
    int GetNBSpeechLpbkParamFromNVRam(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk) {
        ALOGD("%s()", __FUNCTION__);

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)||(!(defined(MTK_AUDIO_GAIN_TABLE)&&defined(MTK_AUDIO_SPH_LPBK_PARAM)))
        (void)pSphParamNBLpbk;
        return 0;
#else
        int result = 0;
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_SPEECH_LPBK_PARAM_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pSphParamNBLpbk) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultSpeechNBLpbkParam(pSphParamNBLpbk);
        // get from NV ram and replace the default value
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        result = read(audio_nvram_fd.iFileDesc, pSphParamNBLpbk , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != rec_size) {
            ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));
            result = getDefaultSpeechNBLpbkParam(pSphParamNBLpbk);
        }
#endif
        return result;
#endif
    }

    int SetNBSpeechLpbkParamToNVRam(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk) {
        ALOGD("%s()", __FUNCTION__);
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)||(!(defined(MTK_AUDIO_GAIN_TABLE)&&defined(MTK_AUDIO_SPH_LPBK_PARAM)))
        (void)pSphParamNBLpbk;
        return 0;
#else
        int result = 0;
        if (!pSphParamNBLpbk) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        result = 0;
#else
        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_SPEECH_LPBK_PARAM_LID;
        int rec_size, rec_num;

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        result = write(audio_nvram_fd.iFileDesc, pSphParamNBLpbk , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

#endif
        return result;
#endif
    }

    // get BT Gain parameter from NVRAM
    int getDefaultAudioBTGain(AUDIO_BT_GAIN_STRUCT *pParaBT) {
        // only for startup use
        ALOGW("sizeof AUDIO_BT_GAIN_STRUCT = %zu", sizeof(AUDIO_BT_GAIN_STRUCT));
        memcpy((void *)pParaBT, (void *)&bt_gain_control_default, sizeof(AUDIO_BT_GAIN_STRUCT));
        return sizeof(AUDIO_BT_GAIN_STRUCT);
    }

    // Speech Loopback parameters
    int GetAudioBTGainParamFromNV(AUDIO_BT_GAIN_STRUCT *pParaBT) {
        ALOGD("%s()", __FUNCTION__);
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)||(!(defined(MTK_AUDIO_GAIN_TABLE)&&defined(MTK_AUDIO_GAIN_TABLE_BT)))
        (void)pParaBT;
        return 0;
#else
        int result = 0;
        F_ID audio_nvram_fd ;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_BT_GAIN_CUSTOM_LID;
        int rec_size, rec_num;
        if (!checkNvramReady()) {
            ALOGW("checkNvramReady fail");
            return 0;
        } else if (!pParaBT) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        // a default value , should disable when NVRAM ready
        result = getDefaultAudioBTGain(pParaBT);
        // get from NV ram and replace the default value
#else
        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        result = read(audio_nvram_fd.iFileDesc, pParaBT , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

        if (result != sizeof(AUDIO_BT_GAIN_STRUCT)) {
            ALOGD("%s(), size wrong, using default parameters,result=%d, struct size=%d", __FUNCTION__, result, sizeof(AUDIO_BT_GAIN_STRUCT));
            result = getDefaultAudioBTGain(pParaBT);
        }
#endif
        return result;
#endif
    }

    int SetAudioBTGainParamToNV(AUDIO_BT_GAIN_STRUCT *pParaBT) {
        ALOGD("%s()", __FUNCTION__);
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)||(!(defined(MTK_AUDIO_GAIN_TABLE)&&defined(MTK_AUDIO_GAIN_TABLE_BT)))
        (void)pParaBT;
        return 0;
#else
        int result = 0;
        if (!pParaBT) {
            ALOGW("%s null ptr", __FUNCTION__);
            return 0;
        }
#if defined(USE_DEFAULT_CUST_TABLE)
        result = 0;
#else
        // write to NV ram
        F_ID audio_nvram_fd;
        int file_lid = AP_CFG_RDCL_FILE_AUDIO_BT_GAIN_CUSTOM_LID;
        int rec_size, rec_num;

        audio_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
        result = write(audio_nvram_fd.iFileDesc, pParaBT , rec_size * rec_num);
        NVM_CloseFileDesc(audio_nvram_fd);

#endif
        return result;
#endif
    }

}
