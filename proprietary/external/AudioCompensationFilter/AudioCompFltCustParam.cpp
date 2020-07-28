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
*   This file implements customized parameter handling.
*   System lib doesn't access XML and NvRam after OS_O
*   Vendor lib accesses XML and NvRam after OS_O
*/

#define LOG_TAG "AudioCompFltCustParam"
#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "unistd.h"
#include "pthread.h"
#endif

#include <utils/Log.h>
#include <utils/String8.h>
#include <cutils/properties.h>
#include "AudioCompFltCustParam.h" // CFG_AUDIO_File.h is used by both of system and vendor
#if defined(SYS_IMPL)
#include <media/AudioSystem.h>
#else
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "CFG_Audio_Default.h"
#endif
#include "AudioParamParser.h"

//#define USE_DEFAULT_CUST_TABLE // For BringUp usage

#ifndef ASSERT
#define ASSERT(x)
#endif

namespace android {

const char *kFltAudioTypeFileName[] = {"PlaybackACF", "PlaybackHCF", "", "", "", "PlaybackDRC", "PlaybackDRC"};
const char* audioHierarchicalParamString = "MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT";
#if defined(SYS_IMPL)
const char *kStringUpdateFLT[] = {"UpdateACFHCFParameters=0", "UpdateACFHCFParameters=1", "", "", "UpdateACFHCFParameters=2", "UpdateACFHCFParameters=3", "UpdateACFHCFParameters=4"};
const char* besLoudRunWithHalString = "MTK_BESLOUDNESS_RUN_WITH_HAL";
#else
const char *kFltCategoryName[] = {"Profile,Speaker", "Profile,Headset", "", "", "", "Volume type,Music", "Volume type,Ring"};
#endif

#define BES_LOUDNESS_L_HFP_FC "bes_loudness_L_hpf_fc"
#define BES_LOUDNESS_L_HPF_ORDER "bes_loudness_L_hpf_order"
#define BES_LOUDNESS_L_LPF_FC "bes_loudness_L_lpf_fc"
#define BES_LOUDNESS_L_LPF_ORDER "bes_loudness_L_lpf_order"
#define BES_LOUDNESS_L_BPF_FC "bes_loudness_L_bpf_fc"
#define BES_LOUDNESS_L_BPF_BW "bes_loudness_L_bpf_bw"
#define BES_LOUDNESS_L_BPF_GAIN "bes_loudness_L_bpf_gain"

#define BES_LOUDNESS_R_HFP_FC "bes_loudness_R_hpf_fc"
#define BES_LOUDNESS_R_HPF_ORDER "bes_loudness_R_hpf_order"
#define BES_LOUDNESS_R_LPF_FC "bes_loudness_R_lpf_fc"
#define BES_LOUDNESS_R_LPF_ORDER "bes_loudness_R_lpf_order"
#define BES_LOUDNESS_R_BPF_FC "bes_loudness_R_bpf_fc"
#define BES_LOUDNESS_R_BPF_BW "bes_loudness_R_bpf_bw"
#define BES_LOUDNESS_R_BPF_GAIN "bes_loudness_R_bpf_gain"

#define BES_LOUDNESS_SEP_LR_FILTER "bes_loudness_Sep_LR_Filter"
#define BES_LOUDNESS_WS_GAIN_MAX "bes_loudness_WS_Gain_Max"
#define BES_LOUDNESS_WS_GAIN_MIN "bes_loudness_WS_Gain_Min"
#define BES_LOUDNESS_FILTER_FIRST "bes_loudness_Filter_First"
#define BES_LOUDNESS_NUM_BANDS "bes_loudness_Num_Bands"
#define BES_LOUDNESS_FLT_BANK_ORDER "bes_loudness_Flt_Bank_Order"
#define BES_LOUDNESS_CROSS_FREQ "bes_loudness_Cross_Freq"
#define DRC_TH "DRC_Th"
#define DRC_GN "DRC_Gn"
#define SB_GN "SB_Gn"
#define SB_MODE "SB_Mode"
#define DRC_DELAY "DRC_Delay"
#define ATT_TIME "Att_Time"
#define REL_TIME "Rel_Time"
#define HYST_TH "Hyst_Th"
#define LIM_TH "Lim_Th"
#define LIM_GN "Lim_Gn"
#define LIM_CONST "Lim_Const"
#define LIM_DELAY "Lim_Delay"
#define SWIPREV "SWIPRev"

#if defined(SYS_IMPL)
int getDefaultAudioCompFltParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam) {
    (void) audioParam;
    ALOGE("[Err] System process should not call this function [%s] eFLTtype [%d]", __FUNCTION__, eFLTtype);
    return 0;
}

void callbackAudioCompFltCustParamXmlChanged(AppHandle *appHandle, const char *audioTypeName) {
    // reload XML file
    AppOps* appOps = appOpsGetInstance();
    (void) appHandle;
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return;
    }

    if (strcmp(audioTypeName, kFltAudioTypeFileName[AUDIO_COMP_FLT_DRC_FOR_MUSIC]) == 0) {
        // "PlaybackDRC"
        ALOGD("PlaybackDRC:UpdateACFHCFParameters=3 +");
        AudioSystem::setParameters(0, String8(kStringUpdateFLT[AUDIO_COMP_FLT_DRC_FOR_MUSIC]));
        ALOGD("PlaybackDRC:UpdateACFHCFParameters=3 -");
    }

    ALOGD("-%s(), audioType = %s", __FUNCTION__, audioTypeName);
}

int audioComFltCustParamInit(void) {
    bool besLoudRunWithHalEnable = appIsFeatureOptionEnabled(besLoudRunWithHalString);
    bool audioHierarchicalParamEnable = appIsFeatureOptionEnabled(audioHierarchicalParamString);
    ALOGD("besLoudRunWithHalEnable [%d], audioHierarchicalParamEnable [%d]", besLoudRunWithHalEnable, audioHierarchicalParamEnable);
    if (!besLoudRunWithHalEnable && audioHierarchicalParamEnable) {
        AppOps* appOps = appOpsGetInstance();
        if (appOps == NULL) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            ASSERT(0);
            return -1;
        }

        AppHandle *pAppHandle = appOps->appHandleGetInstance();
        if (NULL == pAppHandle) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            return -1;
        }
        appOps->appHandleRegXmlChangedCb(pAppHandle, callbackAudioCompFltCustParamXmlChanged);
        ALOGD("audioComFltCustParamInit - callbackAudioCompFltCustParamXmlChanged");
    } else {
        ALOGD("audioComFltCustParamInit - Do nothing");
    }
    return 0;
}
#else
int getDefaultAudioCompFltParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam) {
    int dataSize = 0;
    if (AUDIO_COMP_FLT_AUDIO == eFLTtype) {
        memcpy((void *)audioParam, (void *) & (audio_custom_default), sizeof(audio_custom_default));
        dataSize = sizeof(audio_custom_default);
    } else if (AUDIO_COMP_FLT_HEADPHONE == eFLTtype) {
        memcpy((void *)audioParam, (void *) & (audio_hcf_custom_default), sizeof(audio_hcf_custom_default));
        dataSize = sizeof(audio_hcf_custom_default);
    } else if (AUDIO_COMP_FLT_AUDENH == eFLTtype) {
        memset((void *)audioParam, 0x00, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        dataSize = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    } else if (AUDIO_COMP_FLT_VIBSPK == eFLTtype) {
        memcpy((void *)audioParam, (void *) & (audio_vibspk_custom_default), sizeof(audio_vibspk_custom_default));
        dataSize = sizeof(audio_vibspk_custom_default);
    } else if (AUDIO_COMP_FLT_DRC_FOR_MUSIC == eFLTtype) {
        memcpy((void *)audioParam, (void *) & (audio_musicdrc_custom_default), sizeof(audio_musicdrc_custom_default));
        dataSize = sizeof(audio_musicdrc_custom_default);
    } else if (AUDIO_COMP_FLT_DRC_FOR_RINGTONE == eFLTtype) {
        memcpy((void *)audioParam, (void *) & (audio_ringtonedrc_custom_default), sizeof(audio_ringtonedrc_custom_default));
        dataSize = sizeof(audio_ringtonedrc_custom_default);
    }
    else {
        ASSERT(0);
    }
    return dataSize;
}

int audioComFltCustParamInit(void) {
    return 0;
}
int getPlaybackPostProcessParameterFromXML(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene) {

    int returnValue = 0;

    if (eFLTtype != AUDIO_COMP_FLT_AUDIO
        && eFLTtype != AUDIO_COMP_FLT_HEADPHONE
        && eFLTtype != AUDIO_COMP_FLT_DRC_FOR_MUSIC
        && eFLTtype != AUDIO_COMP_FLT_DRC_FOR_RINGTONE) {
        ALOGE("Error %s Line %d eFLTtype %d", __FUNCTION__, __LINE__, eFLTtype);
        return -1;
    } else {
        ALOGD("%s Type/Name [%d]/[%s]", __FUNCTION__, eFLTtype, kFltAudioTypeFileName[eFLTtype]);

        AppOps* appOps = appOpsGetInstance();
        if (NULL == appOps) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            return -1;
        }

        AppHandle *pAppHandle = appOps->appHandleGetInstance();
        if (NULL == pAppHandle) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            return -1;
        }
        AudioType *pAudioType = appOps->appHandleGetAudioTypeByName(pAppHandle, kFltAudioTypeFileName[eFLTtype]);
        if (NULL == pAudioType) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            return -1;
        }

        appOps->audioTypeReadLock(pAudioType, __FUNCTION__);
        // Load data
        do {
            Param  *pParamInfo;
            uint16_t sizeByteParam;
            ParamUnit *pParamUnit;
            // ParamUnit *pParamUnit = appOps->audioTypeGetParamUnit(pAudioType, kFltCategoryName[eFLTtype]);
            CategoryType* categoryType = appOps->audioTypeGetCategoryTypeByName(pAudioType, "Scene");
            bool hasSceneParameterSupport = (pAudioType && categoryType != NULL);
            if (hasSceneParameterSupport) {
                char scene[128];
                memset(scene, 0, sizeof(scene));
                strncpy(scene, "Scene,", 6);
                if (custScene == NULL || !strcmp(custScene, "")) {
                    strncat(scene, "Default,", 8);
                } else {
                    // Scene,App1,Profile,Speaker
                    strncat(scene, custScene, strlen(custScene));
                    strncat(scene, ",", 1);
                    ALOGD("Caller wants to load %s", scene);
                    Category *category = appOps->categoryTypeGetCategoryByName(categoryType, custScene);
                    if (NULL == category) {
                        ALOGD("Scene [%s] not found, use default scene", custScene);
                        // use default scene
                        memset(scene, 0, sizeof(scene));
                        strncpy(scene, "Scene,Default,", 14) ;
                    }
                }
                strncat(scene, kFltCategoryName[eFLTtype], strlen(kFltCategoryName[eFLTtype]));
                pParamUnit = appOps->audioTypeGetParamUnit(pAudioType, scene);
                ALOGD("Load from scene %s", scene);
            } else {
                pParamUnit = appOps->audioTypeGetParamUnit(pAudioType, kFltCategoryName[eFLTtype]);
            }
            if (NULL == pParamUnit) {
                returnValue = -1;
                ALOGE("Error %s %d", __FUNCTION__, __LINE__);
                break;
            }
            /* L Filter */
            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_HFP_FC);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_HPF_ORDER);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_order = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_LPF_FC);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_LPF_ORDER);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_order = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_BPF_FC);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->bes_loudness_f_param.V5F.bes_loudness_L_bpf_fc), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_BPF_BW);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->bes_loudness_f_param.V5F.bes_loudness_L_bpf_bw), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_BPF_GAIN);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->bes_loudness_f_param.V5F.bes_loudness_L_bpf_gain), pParamInfo->data, sizeByteParam);
            /* R Filter */
            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_HFP_FC);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_R_hpf_fc = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_HPF_ORDER);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_R_hpf_order = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_LPF_FC);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_LPF_ORDER);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_order = *((unsigned int *)pParamInfo->data);
            ALOGD("L/R_hpf_fc = %d/%d, L/R_hpf_order = %d/%d, L/R_lpf_fc = %d/%d, L/R_lpf_order = %d/%d",
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc,
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_R_hpf_fc,
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_order,
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_R_hpf_order,
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc,
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc,
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_order,
                  audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_order);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_BPF_FC);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->bes_loudness_f_param.V5F.bes_loudness_R_bpf_fc), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_BPF_BW);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->bes_loudness_f_param.V5F.bes_loudness_R_bpf_bw), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_BPF_GAIN);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->bes_loudness_f_param.V5F.bes_loudness_R_bpf_gain), pParamInfo->data, sizeByteParam);
            /*DRC*/
            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_SEP_LR_FILTER);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_Sep_LR_Filter = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_WS_GAIN_MAX);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_WS_Gain_Max = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_WS_GAIN_MIN);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_WS_Gain_Min = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_FILTER_FIRST);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_Filter_First = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_NUM_BANDS);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_Num_Bands = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_FLT_BANK_ORDER);
            ASSERT(pParamInfo != NULL);
            audioParam->bes_loudness_Flt_Bank_Order = *((unsigned int *)pParamInfo->data);
            ALOGD("Sep_LR_Filter = %d, WS_Gain_Max = %d, WS_Gain_Min = %d, Filter_First = %d, Num_Bands = %d, Flt_Bank_Order = %d",
                  audioParam->bes_loudness_Sep_LR_Filter, audioParam->bes_loudness_WS_Gain_Max, audioParam->bes_loudness_WS_Gain_Min,
                  audioParam->bes_loudness_Filter_First, audioParam->bes_loudness_Num_Bands, audioParam->bes_loudness_Flt_Bank_Order);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_CROSS_FREQ);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->bes_loudness_Cross_Freq), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, DRC_TH);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->DRC_Th), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, DRC_GN);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->DRC_Gn), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, SB_GN);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->SB_Gn), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, SB_MODE);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->SB_Mode), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, DRC_DELAY);
            ASSERT(pParamInfo != NULL);
            audioParam->DRC_Delay = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, ATT_TIME);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->Att_Time), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, REL_TIME);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->Rel_Time), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, HYST_TH);
            ASSERT(pParamInfo != NULL);
            sizeByteParam = appOps->paramGetNumOfBytes(pParamInfo);
            memcpy(&(audioParam->Hyst_Th), pParamInfo->data, sizeByteParam);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, LIM_TH);
            ASSERT(pParamInfo != NULL);
            audioParam->Lim_Th = *((int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, LIM_GN);
            ASSERT(pParamInfo != NULL);
            audioParam->Lim_Gn = *((int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, LIM_CONST);
            ASSERT(pParamInfo != NULL);
            audioParam->Lim_Const = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, LIM_DELAY);
            ASSERT(pParamInfo != NULL);
            audioParam->Lim_Delay = *((unsigned int *)pParamInfo->data);

            pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, SWIPREV);
            ASSERT(pParamInfo != NULL);
            audioParam->SWIPRev = *((int *)pParamInfo->data);
            ALOGD("DRC_Delay = %d, Lim_Th = %d, Lim_Gn = %d, Lim_Const = %d, Lim_Delay = %d, SWIPRev = 0x%x",
                  audioParam->DRC_Delay, audioParam->Lim_Th, audioParam->Lim_Gn, audioParam->Lim_Const,
                  audioParam->Lim_Delay, audioParam->SWIPRev);
        } while (0);
        appOps->audioTypeUnlock(pAudioType);
    }
    if (returnValue < 0) {
        ALOGD("%s Parameter %d returnValue %d", __FUNCTION__, eFLTtype, returnValue);
    }
    return returnValue;
}
#endif

extern "C" int  getAudioCompFltCustParamFromStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene) {
    int result = 0;
#if !defined(SYS_IMPL)
#if defined(USE_DEFAULT_CUST_TABLE)
    (void) custScene;
    result = getDefaultAudioCompFltParam(eFLTtype, audioParam);
#else
    bool audioHierarchicalParamEnable = appIsFeatureOptionEnabled(audioHierarchicalParamString);
    if (audioHierarchicalParamEnable) {
        if (eFLTtype == AUDIO_COMP_FLT_AUDIO
            || eFLTtype == AUDIO_COMP_FLT_HEADPHONE
            || eFLTtype == AUDIO_COMP_FLT_DRC_FOR_MUSIC
            || eFLTtype == AUDIO_COMP_FLT_DRC_FOR_RINGTONE) {
            if (getPlaybackPostProcessParameterFromXML(eFLTtype, audioParam, custScene) >= 0) {
                result = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
            } else {
                result = 0;
            }

        }
    } else {
        F_ID audioNvramFileID;
        int fileLID;
        int recordSize, recordNum;
        if (AUDIO_COMP_FLT_AUDIO == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
        } else if (AUDIO_COMP_FLT_HEADPHONE == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID;
        } else if (AUDIO_COMP_FLT_VIBSPK == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_VIBSPK_COMPFLT_LID;
        } else if (AUDIO_COMP_FLT_DRC_FOR_MUSIC == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_AUDIO_MUSIC_DRC_LID;
        } else if (AUDIO_COMP_FLT_DRC_FOR_RINGTONE == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_AUDIO_RINGTONE_DRC_LID;
        } else { // Shouldn't happen
            fileLID = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
            ASSERT(0);
        }
            audioNvramFileID = NVM_GetFileDesc(fileLID, &recordSize, &recordNum, ISREAD);
            result = read(audioNvramFileID.iFileDesc, audioParam, recordSize * recordNum);
            NVM_CloseFileDesc(audioNvramFileID);
    }
    if (result != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT)) {
        ALOGE("%s(), size wrong, using default parameters,result=%d, struct size=%zu", __FUNCTION__, result, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        result = getDefaultAudioCompFltParam(eFLTtype, audioParam);
    }
#endif
#else
    (void) audioParam;
    (void) custScene;
    ALOGE("[Err] System process should not call this function [%s] eFLTtype [%d]", __FUNCTION__, eFLTtype);
#endif
    return result;
}

extern "C" int  setAudioCompFltCustParamToStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam) {
    int result = 0;
#if !defined(SYS_IMPL)
#if defined(USE_DEFAULT_CUST_TABLE)
    (void) eFLTtype;
    (void) audioParam;
    result = 0;
#else
    bool audioHierarchicalParamEnable = appIsFeatureOptionEnabled(audioHierarchicalParamString);
    if (!audioHierarchicalParamEnable) {
        // write to NV ram
        F_ID audioNvramFileID;
        int fileLID;
        int recordSize, recordNum;

        if (AUDIO_COMP_FLT_AUDIO == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
        } else if (AUDIO_COMP_FLT_HEADPHONE == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID;
        } else if (AUDIO_COMP_FLT_VIBSPK == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_VIBSPK_COMPFLT_LID;
        } else if (AUDIO_COMP_FLT_DRC_FOR_MUSIC == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_AUDIO_MUSIC_DRC_LID;
        } else if (AUDIO_COMP_FLT_DRC_FOR_RINGTONE == eFLTtype) {
            fileLID = AP_CFG_RDCL_FILE_AUDIO_RINGTONE_DRC_LID;
        } else { //Shouldn't happen
            fileLID = AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID;
            ASSERT(0);
        }

        audioNvramFileID = NVM_GetFileDesc(fileLID, &recordSize, &recordNum, ISWRITE);
        result = write(audioNvramFileID.iFileDesc, audioParam, recordSize * recordNum);
        NVM_CloseFileDesc(audioNvramFileID);
    }
#endif
#else
    (void) audioParam;
    ALOGE("[Err] System process should not call this function [%s] eFLTtype [%d]", __FUNCTION__, eFLTtype);
#endif
    return result;
}

}; // namespace android
