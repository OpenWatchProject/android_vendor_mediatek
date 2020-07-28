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
 * AudioCompFltCustParamc.c
 *
 * Project:
 * --------
 *   Aurisys
 *
 * Description:
 * ------------
 *   This file implements customized parameter handling in C
 *
 * Author:
 * -------
 *   Liang
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 *******************************************************************************/

/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#define LOG_TAG "AudioCompFltCustParam"
#include <string.h>
#include "unistd.h"
#include <utils/Log.h>
#include "CFG_AUDIO_File.h"
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "AudioCompFltCustParamc.h"
#include "CFG_Audio_Default.h"
#include <cutils/properties.h>

#include "AudioParamParser.h"

#ifdef MTK_BASIC_PACKAGE
#define USE_DEFAULT_CUST_TABLE //For BringUp usage
#endif

#ifndef ASSERT
#define ASSERT(x)
#endif

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
    } else {
        ASSERT(0);
    }
    return dataSize;
}

const char *kFltAudioTypeFileName[] = {"PlaybackACF", "PlaybackHCF", "", "", "", "PlaybackDRC", "PlaybackDRC",
                                       "PlaybackHCF", "PlaybackACF"
                                      };
/*enable ACF Ringtone Tuning by set last item from "Volume type,Music,Profile,Speaker" to "Volume type,Ring,Profile,Speaker" */
const char *kFltCategoryName[] = {"Volume type,Music,Profile,Speaker", "Profile,Headset", "", "", "", "Volume type,Music", "Volume type,Ring",
                                  "Profile,USB", "Volume type,Music,Profile,Speaker"
                                 };
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

const char* audioHierarchicalParamString = "MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT";


int getCallbackFilter(AudioCompFltType_t eFLTtype) {
    int ret = 0;
    if ((mFilterType & AUDIO_COMP_FLT_XML_AUDIO) && eFLTtype == AUDIO_COMP_FLT_AUDIO) {
        ret = 1;
    } else if ((mFilterType & AUDIO_COMP_FLT_XML_HEADPHONE) && eFLTtype == AUDIO_COMP_FLT_HEADPHONE) {
        ret = 1;
    } else if ((mFilterType & AUDIO_COMP_FLT_XML_DRC_FOR_MUSIC) && eFLTtype == AUDIO_COMP_FLT_DRC_FOR_MUSIC) {
        ret = 1;
    } else if ((mFilterType & AUDIO_COMP_FLT_XML_HCF_USB) && eFLTtype == AUDIO_COMP_FLT_HCF_FOR_USB) {
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}

void resetCallbackFilter() {

    mFilterType = AUDIO_COMP_FLT_XML_NONE;
}

int getPlaybackPostProcessParameterFromXML(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene) {

    int returnValue = 0;

    if (eFLTtype != AUDIO_COMP_FLT_AUDIO
        && eFLTtype != AUDIO_COMP_FLT_HEADPHONE
        && eFLTtype != AUDIO_COMP_FLT_HCF_FOR_USB
        && eFLTtype != AUDIO_COMP_FLT_DRC_FOR_MUSIC
        && eFLTtype != AUDIO_COMP_FLT_DRC_FOR_RINGTONE
        && eFLTtype != AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE) {
        ALOGE("Error %s Line %d eFLTtype %d", __FUNCTION__, __LINE__, eFLTtype);
        return -1;
    } else {
        ALOGD("%s Type/Name [%d]/[%s]", __FUNCTION__, eFLTtype, kFltAudioTypeFileName[eFLTtype]);

        AppOps *appOps = appOpsGetInstance();
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

        Param  *pParamInfo;
        ParamUnit *pParamUnit;
        uint16_t sizeByteParam;
        CategoryType* categoryType = appOps->audioTypeGetCategoryTypeByName(pAudioType, "Scene");
        bool hasSceneParameterSupport = (pAudioType && categoryType != NULL);
        ALOGV("hasSceneParameterSupport %d", hasSceneParameterSupport);
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
            appOps->audioTypeUnlock(pAudioType);
            return returnValue;
        }
        /* L Filter */
        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_HFP_FC);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_L_hpf_fc = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_HPF_ORDER);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_order = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_L_hpf_order = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_L_hpf_order);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_LPF_FC);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_L_lpf_fc = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_L_LPF_ORDER);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_order = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_L_lpf_order = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_L_lpf_order);

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
        ALOGV("bes_loudness_R_hpf_fc = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_R_hpf_fc);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_HPF_ORDER);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_f_param.V5F.bes_loudness_R_hpf_order = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_R_hpf_order = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_R_hpf_order);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_LPF_FC);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_R_lpf_fc = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_R_LPF_ORDER);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_order = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_R_lpf_order = %d", audioParam->bes_loudness_f_param.V5F.bes_loudness_R_lpf_order);

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
        ALOGV("bes_loudness_Sep_LR_Filter = %d", audioParam->bes_loudness_Sep_LR_Filter);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_WS_GAIN_MAX);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_WS_Gain_Max = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_WS_Gain_Max = %d", audioParam->bes_loudness_WS_Gain_Max);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_WS_GAIN_MIN);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_WS_Gain_Min = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_WS_Gain_Min = %d", audioParam->bes_loudness_WS_Gain_Min);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_FILTER_FIRST);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_Filter_First = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_Filter_First = %d", audioParam->bes_loudness_Filter_First);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_NUM_BANDS);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_Num_Bands = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_Num_Bands = %d", audioParam->bes_loudness_Num_Bands);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, BES_LOUDNESS_FLT_BANK_ORDER);
        ASSERT(pParamInfo != NULL);
        audioParam->bes_loudness_Flt_Bank_Order = *((unsigned int *)pParamInfo->data);
        ALOGV("bes_loudness_Flt_Bank_Order = %d", audioParam->bes_loudness_Flt_Bank_Order);

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
        ALOGV("DRC_Delay = %d", audioParam->DRC_Delay);

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
        ALOGV("Lim_Th = %d", audioParam->Lim_Th);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, LIM_GN);
        ASSERT(pParamInfo != NULL);
        audioParam->Lim_Gn = *((int *)pParamInfo->data);
        ALOGV("Lim_Gn = %d", audioParam->Lim_Gn);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, LIM_CONST);
        ASSERT(pParamInfo != NULL);
        audioParam->Lim_Const = *((unsigned int *)pParamInfo->data);
        ALOGV("Lim_Const = %d", audioParam->Lim_Const);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, LIM_DELAY);
        ASSERT(pParamInfo != NULL);
        audioParam->Lim_Delay = *((unsigned int *)pParamInfo->data);
        ALOGV("Lim_Delay = %d", audioParam->Lim_Delay);

        pParamInfo = appOps->paramUnitGetParamByName(pParamUnit, SWIPREV);
        ASSERT(pParamInfo != NULL);
        audioParam->SWIPRev = *((int *)pParamInfo->data);
        ALOGV("SWIPRev = 0x%x", audioParam->SWIPRev);
        appOps->audioTypeUnlock(pAudioType);
    }
    ALOGV("%s Parameter %d returnValue %d", __FUNCTION__, eFLTtype, returnValue);
    return returnValue;
}

void callbackAudioCompFltCustParamXmlChanged(AppHandle *appHandle, const char *audioTypeName) {
    ALOGD("+%s(), audioType = %s", __FUNCTION__, audioTypeName);
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return;
    }
    // reload XML file
    if (appOps->appHandleReloadAudioType(appHandle, audioTypeName) == APP_ERROR) {
        ALOGE("%s(), Reload xml fail!(audioType = %s)", __FUNCTION__, audioTypeName);
    } else {
        if (strcmp(audioTypeName, kFltAudioTypeFileName[AUDIO_COMP_FLT_AUDIO]) == 0) {
            //"PlaybackACF"
            ALOGD("PlaybackACF:UpdateACFHCFParameters= ACF +");
            mFilterType |= AUDIO_COMP_FLT_XML_AUDIO;
            ALOGD("PlaybackACF:UpdateACFHCFParameters= ACF -");
        } else if (strcmp(audioTypeName, kFltAudioTypeFileName[AUDIO_COMP_FLT_HEADPHONE]) == 0) {
            //"PlaybackHCF"
            ALOGD("PlaybackHCF:UpdateACFHCFParameters= HCF +");
            mFilterType |= AUDIO_COMP_FLT_XML_HEADPHONE ;
            ALOGD("PlaybackHCF:UpdateACFHCFParameters= HCF -");
        } else if (strcmp(audioTypeName, kFltAudioTypeFileName[AUDIO_COMP_FLT_HCF_FOR_USB]) == 0) {
            //"PlaybackUsbACF"
            ALOGD("PlaybackHCF:UpdateACFHCFParameters= HCF_USB +");
            mFilterType |= AUDIO_COMP_FLT_XML_HCF_USB ;
            ALOGD("PlaybackHCF:UpdateACFHCFParameters= HCF_USB -");
        } else if (strcmp(audioTypeName, kFltAudioTypeFileName[AUDIO_COMP_FLT_DRC_FOR_MUSIC]) == 0) {
            //"PlaybackDRC"
            ALOGD("PlaybackDRC:UpdateACFHCFParameters= DRC +");
            mFilterType |= AUDIO_COMP_FLT_XML_DRC_FOR_MUSIC;
            ALOGD("PlaybackDRC:UpdateACFHCFParameters= DRC -");
        }
    }
    ALOGD("-%s(), audioType = %s", __FUNCTION__, audioTypeName);
}
int audioComFltCustParamInit(void) {
    ALOGD("audioComFltCustParamInit + ");
    AppOps *appOps = appOpsGetInstance();
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
    return 0;
}

int  getAudioCompFltCustParamFromStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene) {
    int result = 0;

#if defined(USE_DEFAULT_CUST_TABLE)
    result = getDefaultAudioCompFltParam(eFLTtype, audioParam);
#else
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

    bool audioHierarchicalParamEnable = appIsFeatureOptionEnabled(audioHierarchicalParamString);
    ALOGD("audioHierarchicalParamEnable [%d]", audioHierarchicalParamEnable);
    if (audioHierarchicalParamEnable && (eFLTtype == AUDIO_COMP_FLT_AUDIO
        || eFLTtype == AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE
        || eFLTtype == AUDIO_COMP_FLT_HEADPHONE
        || eFLTtype == AUDIO_COMP_FLT_HCF_FOR_USB
        || eFLTtype == AUDIO_COMP_FLT_DRC_FOR_MUSIC
        || eFLTtype == AUDIO_COMP_FLT_DRC_FOR_RINGTONE)) {
        if (getPlaybackPostProcessParameterFromXML(eFLTtype, audioParam, custScene) >= 0) {
            result = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
        } else {
            result = 0;
        }

    } else {
        audioNvramFileID = NVM_GetFileDesc(fileLID, &recordSize, &recordNum, ISREAD);
        result = read(audioNvramFileID.iFileDesc, audioParam, recordSize * recordNum);
        NVM_CloseFileDesc(audioNvramFileID);
    }

    if (result != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT)) {
        ALOGE("%s(), size wrong, using default parameters,result=%d, struct size=%zu", __FUNCTION__,
              result, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        result = getDefaultAudioCompFltParam(eFLTtype, audioParam);
    }
#endif
    return result;
}

int  setAudioCompFltCustParamToStorage(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam) {
    int result = 0;
#if defined(USE_DEFAULT_CUST_TABLE)
    result = 0;
#else
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
#endif
    return result;
}

int getDRCControlOptionParamFromStorage(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara) {
    char property_value[PROPERTY_VALUE_MAX];
    property_get("persist.vendor.audiohal.besloudness_state", property_value, "1");
    int besloudness_enable = atoi(property_value);
    if (besloudness_enable) {
        pPara->u32EnableFlg = 1;
    } else {
        pPara->u32EnableFlg = 0;
    }
    return 1;
}


