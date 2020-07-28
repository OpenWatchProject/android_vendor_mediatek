/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
 * AudioCmdHandler.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the handling about audio command comming from AT Command Service.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 * 07 15 2013 weiguo.li
 * [ALPS00837980] [MakeFile] [Remove: Feature Option] MTK_TVOUT_SUPPORT
 * .
 *
 * 05 15 2013 donglei.ji
 * [ALPS00683353] [Need Patch] [Volunteer Patch] DMNR3.0 and VOIP tuning check in
 * .
 *
 * 01 20 2013 donglei.ji
 * [ALPS00448366] [Need Patch] [Volunteer Patch] MM Command Handler add command to get chip info
 * .
 *
 * 01 09 2013 donglei.ji
 * [ALPS00438595] [Need Patch] [Volunteer Patch] Audio Tool -  HD Calibration
 * HD Record Calibration.
 *
 * 08 15 2012 donglei.ji
 * [ALPS00337843] [Need Patch] [Volunteer Patch] MM Command Handler JB migration
 * MM Command Handler JB migration.
 *
 * 06 28 2012 donglei.ji
 * [ALPS00308450] [Need Patch] [Volunteer Patch] Audio Taste feature
 * Audio Taste feature.
 *
 * 06 27 2012 donglei.ji
 * [ALPS00307929] [Need Patch] [Volunteer Patch] Audio HQA add I2S output 32 bit test case
 * for MT6577 HQA.
 *
 * 04 27 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * add two case:
 * 1. get phone support info
 * 2. read dual mic parameters if phone support dual mic
 *
 * 04 20 2012 donglei.ji
 * [ALPS00272538] [Need Patch] [Volunteer Patch] AT Command for ACF/HCF calibration
 * ACF/HCF calibration feature.
 *
 * 04 12 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * Add wide band speech tuning.
 *
 * 03 16 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * add speech parameters encode and decode for transfer.
 *
 * 03 09 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * add a set parameters setting.
 *
 * 03 06 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * AT Command for Speech Tuning tool feature check in.
 *
 * 12 27 2011 donglei.ji
 * [ALPS00107090] [Need Patch] [Volunteer Patch][ICS Migration] MM Command Handler Service Migration
 * MM Command Handler Service check in.
 *
 * 11 21 2011 donglei.ji
 * [ALPS00094843] [Need Patch] [Volunteer Patch] XLOG enhance
 * log enhance -- SXLOG.
 *
 * 07 15 2011 donglei.ji
 * [ALPS00053673] [Need Patch] [Volunteer Patch][Audio HQA]Add Audio HQA test cases
 * check in HQA code.
 *
 * 06 15 2011 donglei.ji
 * [ALPS00053673] [Need Patch] [Volunteer Patch][Audio HQA]Add Audio HQA test cases
 * MM Cmd Handler code check in for MT6575.
 *
 *******************************************************************************/

/*=============================================================================
 *                              Include Files
 *===========================================================================*/

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <log/log.h>
#include <cutils/properties.h>
#include <utils/String8.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <netinet/in.h>
#include <pthread.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <system/audio.h>

#include "AudioCmdHandler.h"
#include "Parameter.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioCmdHandlerService"

#define SOCKET_NAME_AUDIO "atci-audio"
#define MAX_BUFSIZE_RECV 4*1024
#define MAX_CMD_LEN 128
static const char kRespFail[]  = " Fail";
static const char kRespOK[] = " OK";
static size_t kRespFailLen = strlen(kRespFail);
static size_t kRespOKLen = strlen(kRespOK);


#ifdef DISABLE_AUDIO_DAEMON_IN_USER_LOAD
#include <cutils/properties.h>
#define BUILD_TYPE_PROP "ro.build.type"
#define BUILD_TYPE_PROP_ENG "eng"
#define BUILD_TYPE_PROP_USER "user"
#define IS_BUILDTYPE(a,b) ((strcmp(a,b)==0)?true:false)
#endif
static int s_fdAudioCmdHdlSrv_listen  = -1;
static int s_fdAudioCmdHdlSrv_command = -1;

#define MAX_BLOCK_NUM 10
using namespace android;
typedef enum
{
    UNKNOWN_CMD = -1,
    RDPRM_CMD,
    RT_VOL_CMD,
    EVOL_CMD,
    EADP_CMD,
    EAPS_CMD,
    EMAGI_CMD,
    EHAC_CMD,
    ELPBK_CMD,
    EDISPBTNREC_CMD,
    EBTGAIN_CMD,
    EACF_CMD,
    EHCF_CMD,
    EDUALMIC_CMD,
    EDMNRPLYREC_CMD,
    EDUALMICGAIN_CMD,
    EHDREC_CMD,
    ERECORD_CMD,
    GPSI_CMD,
    EAUDT_CMD,
    GCVI_CMD,
    EVOIP_CMD,
    //--->Add for speech tuning tool
    EDISPITEM_CMD,
    EHFP_CMD,
    EGAIN_CMD
} AudioCmdType;

//********* function declaration********************//
static void sendResponseToATCI(const char *pSendData, int length);

template <class... T> void unused(T&&...)
{}

//********* function definition************************//
static void DoSetRecorderParam(AudioCmdParam &audioCmdParams, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHParamError;
    if ((pAudioHandle != NULL) && (audioCmdParams.recvDataLen >= strlen("AUD+RDPRM=1,1,1")))
    {
        ret = pAudioHandle->setRecorderParam(audioCmdParams);
    }

    if (ret != ACHSucceeded)
    {
        sendResponseToATCI("AUD+RDPRM=Failed,params error", strlen("AUD+RDPRM=Failed,params error"));
    }
    else
    {
        sendResponseToATCI("AUD+RDPRM=Succeeded", strlen("AUD+RDPRM=Succeeded"));
    }
}

static void DoCustSPHMagiParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EMAGI=0");
    char *pSpeechParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];

    if (audioCmdParams.param1 == 0)
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pSpeechParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pSpeechParam, responseStr, strlen(responseStr));

		ret = pAudioHandle->ULCustSPHMagiParamFromNV(pSpeechParam + strLen, &dataLen);
		pSpeechParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pSpeechParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pSpeechParam);
    }
    else
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)pParam + strLen;

		ret = pAudioHandle->DLCustSPHMagiParamToNV(pSpeechParam);
        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}
static void DoCustSPHHACParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EHAC=0");
    char *pSpeechParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];

    if (audioCmdParams.param1 == 0)
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pSpeechParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pSpeechParam, responseStr, strlen(responseStr));

		ret = pAudioHandle->ULCustSPHHACParamFromNV(pSpeechParam + strLen, &dataLen);
		pSpeechParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pSpeechParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pSpeechParam);
    }
    else
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)pParam + strLen;

		ret = pAudioHandle->DLCustSPHHACParamToNV(pSpeechParam);
        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}
static void DoCustSPHLPBKParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+ESLBK=0");
    char *pSpeechParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];

    if (audioCmdParams.param1 == 0)
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pSpeechParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pSpeechParam, responseStr, strlen(responseStr));

		ret = pAudioHandle->ULCustSPHLPBKParamFromNV(pSpeechParam + strLen, &dataLen);
		pSpeechParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pSpeechParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pSpeechParam);
    }
    else
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)pParam + strLen;

		ret = pAudioHandle->DLCustSPHLPBKParamToNV(pSpeechParam);
        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}

static void DoGetVolume(AudioCmdHandler *pAudioHandle)
{
    if (pAudioHandle == NULL)
        return;

    char responseStr[MAX_CMD_LEN];
    int index;
    pAudioHandle->getVolume(&index);
    snprintf(responseStr, MAX_CMD_LEN, "MM+VOL=%d\r\n", index);
    ALOGD("DoGetVolume: %s", responseStr);
    sendResponseToATCI(responseStr, strlen(responseStr));
}

//<--- for speech parameters calibration
static void DoCustSPHParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EAPS=0,0,0");
    char *pSpeechParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];

    if (audioCmdParams.param1 == 0)
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pSpeechParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy(pSpeechParam, responseStr, strlen(responseStr));

        if (audioCmdParams.param2 == 0)
        {
            ret = pAudioHandle->ULCustSPHParamFromNV(pSpeechParam + strLen, &dataLen, audioCmdParams.param3);
        }
        else if (audioCmdParams.param2 == 1)
        {
            ret = pAudioHandle->ULCustSPHWBParamFromNV(pSpeechParam + strLen, &dataLen, audioCmdParams.param3);
        }
		pSpeechParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pSpeechParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pSpeechParam);
    }
    else
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)pParam + strLen;
        if (audioCmdParams.param2 == 0)
        {
            ret = pAudioHandle->DLCustSPHParamToNV(pSpeechParam, audioCmdParams.param3);
        }
        else if (audioCmdParams.param2 == 1)
        {
            ret = pAudioHandle->DLCustSPHWBParamToNV(pSpeechParam, audioCmdParams.param3);
        }

        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}

static void DoCustSPHVolumeParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EADP=0");
    char *pSPHVolumeParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    if (audioCmdParams.param1 == 0)
    {
        pSPHVolumeParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pSPHVolumeParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy(pSPHVolumeParam, responseStr, strlen(responseStr));

        ret = pAudioHandle->ULCustSPHVolumeParamFromNV(pSPHVolumeParam + strLen, &dataLen);
		pSPHVolumeParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pSPHVolumeParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pSPHVolumeParam);
    }
    else
    {
        pSPHVolumeParam = (char *)pParam + strLen;
        ret = pAudioHandle->DLCustSPHVolumeParamToNV(pSPHVolumeParam);

        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}

static void DoCustACFHCFParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = 0;
    char *pACFHCFParam = NULL;

	strLen = sizeof("MM+EACF=0,0");

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }
    // extract command string
    char responseStr[32];

    if (audioCmdParams.param1 == 0)
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pACFHCFParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pACFHCFParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pACFHCFParam, responseStr, strlen(responseStr));

        #if 0
        if (audioCmdParams.param2 == 0)
        {
            ret = pAudioHandle->ULCustACFParamFromNV(pACFHCFParam + strLen, &dataLen);
        }
        else if (audioCmdParams.param2 == 1)
        {
            ret = pAudioHandle->ULCustHCFParamFromNV(pACFHCFParam + strLen, &dataLen);
        }
        #else
            ret = pAudioHandle->ULCustAudioFLTParamFromNV(pACFHCFParam + strLen, &dataLen,(AudioFltTunningType) audioCmdParams.param2);
        #endif
		pACFHCFParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pACFHCFParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pACFHCFParam);
    }
    else
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pACFHCFParam = (char *)pParam + strLen;
#if 0
        if (audioCmdParams.param2 == 0)
        {
            ret = pAudioHandle->DLCustACFParamToNV(pACFHCFParam);
        }
        else if (audioCmdParams.param2 == 1)
        {
            ret = pAudioHandle->DLCustHCFParamToNV(pACFHCFParam);
        }
#else
            ret = pAudioHandle->DLCustAudioFLTParamToNV(pACFHCFParam,(AudioFltTunningType) audioCmdParams.param2);
#endif
        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}

static void DoAudioTaste(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int strLen = sizeof("MM+EAUDT=0");

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    if (audioCmdParams.param1 == 0)
    {
        ret = pAudioHandle->AudioTasteRunning(0);

    }
    else
    {
        ret = pAudioHandle->AudioTasteRunning((unsigned short)audioCmdParams.param1, (char *)pParam + strLen);

    }

    if (ret == ACHSucceeded)
    {
        strncat(responseStr, kRespOK, kRespOKLen);
    }
    else
    {
        strncat(responseStr, kRespFail, kRespFailLen);
    }

    sendResponseToATCI(responseStr, strlen(responseStr));
}

static void DoCustDUALMICParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    char *pDUALMICParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    int strLen = strlen((char *)pParam) + 1;
    if (strLen > 32)
    {
        ALOGE("DoCustDUALMICParam - command is wrong:%s", (char *)pParam);
        return;
    }
    strncpy(responseStr, (char *)pParam, strLen-1);

    if (audioCmdParams.param1 == 0)
    {
        pDUALMICParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pDUALMICParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pDUALMICParam, responseStr, strlen(responseStr));

        ret = pAudioHandle->ULCustDualMicParamFromNV(pDUALMICParam + strLen, &dataLen, audioCmdParams.param2); // 0 for 44+76,1 for VOIP
		pDUALMICParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pDUALMICParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pDUALMICParam);
    }
    else if (audioCmdParams.param1 == 1)
    {
        pDUALMICParam = (char *)pParam + strLen;

        ret = pAudioHandle->DLCustDualMicParamToNV(pDUALMICParam, audioCmdParams.param2); // 1 DMNR parameters for VOIP

        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
    else
    {
        strncat(responseStr, kRespFail, kRespFailLen);
        sendResponseToATCI(responseStr, strlen(responseStr));
    }
}

static void DoCustDualMicNR(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int strLen = sizeof("MM+EDMNRPLYREC=0,1");

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    ret = pAudioHandle->AudioDMNRTuning((unsigned short)audioCmdParams.param1, (bool)audioCmdParams.param2, (char *)pParam + strLen);

    if (ret == ACHSucceeded)
    {
        strncat(responseStr, kRespOK, kRespOKLen);
    }
    else
    {
        strncat(responseStr, kRespFail, kRespFailLen);
    }

    sendResponseToATCI(responseStr, strlen(responseStr));
}

static void DoCustDMNRGainSetting(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int strLen = 0;
    int dataLen = 0;
    char *pDMNRTuning = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    if (strlen((char *)pParam) >= 32)
    {
        return;
    }
    memset(responseStr, 0, 32);
    memcpy(responseStr, (char *)pParam, strlen("MM+EDMNRGSET=0,0"));
    ALOGI ("DoCustDMNRGainSetting in, responseStr= %s", responseStr);

    if (audioCmdParams.param1 == 0)
    {
        pDMNRTuning = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pDMNRTuning == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pDMNRTuning, responseStr, strlen(responseStr));

        strLen = sizeof("MM+EDMNRGSET=0,0");
        ALOGI ("sizeof(MM+EDMNRGSET=0,0) = %d, audioCmdParams.param2=%d", strLen, audioCmdParams.param2);
        ret = pAudioHandle->getDMNRGain(audioCmdParams.param2, pDMNRTuning + strLen, &dataLen);
		pDMNRTuning[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pDMNRTuning, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pDMNRTuning);
    }
    else
    {
        ret = pAudioHandle->setDMNRGain(audioCmdParams.param2, audioCmdParams.param3);

        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }

}

static void DoCustHDRecParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EHDREC=0,0");
    char *pHDRecParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    if (audioCmdParams.param1 == 0)
    {
        pHDRecParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pHDRecParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pHDRecParam, responseStr, strlen(responseStr));
#if 0
        if (audioCmdParams.param2 == 2)
        {
            ret = pAudioHandle->ULCustHDRecSceTableFromNV(pHDRecParam + strLen, &dataLen);
        }
#else
        if (audioCmdParams.param2 == 6)
        {
            ret = pAudioHandle->ULCustHDRecSceTableFromNV(pHDRecParam + strLen, &dataLen);
        }
#endif
        else if (audioCmdParams.param2 == 8)
        {
            ret = pAudioHandle->ULCustHDRecHSParamFromNV(pHDRecParam + strLen, &dataLen); //for 48k sample rate
        }
        else
        {
            int block = (audioCmdParams.param2 > 2) ? (audioCmdParams.param2 - 4) : audioCmdParams.param2;
            ret = pAudioHandle->ULCustHDRecParamFromNV(pHDRecParam + strLen, &dataLen, block);
        }
		pHDRecParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pHDRecParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pHDRecParam);
    }
    else
    {
        pHDRecParam = (char *)pParam + strLen;
#if 0
        if (audioCmdParams.param2 == 2)
        {
            ret = pAudioHandle->DLCustHDRecSceTableToNV(pHDRecParam);
        }
#else
        if (audioCmdParams.param2 == 6)
        {
            ret = pAudioHandle->DLCustHDRecSceTableToNV(pHDRecParam);
        }
#endif
        else if (audioCmdParams.param2 == 8)
        {
            ret = pAudioHandle->DLCustHDRecHSParamToNV(pHDRecParam); //for 48k sample rate
        }
        else
        {
            int block = (audioCmdParams.param2 > 2) ? (audioCmdParams.param2 - 4) : audioCmdParams.param2;
            ret = pAudioHandle->DLCustHDRecParamToNV(pHDRecParam, block);
        }

        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}
#ifdef MTK_AUDIO_GAIN_TABLE
static void DoGetGainInfoForDisp(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EDISPITEM=0,0");
    void *pGainForDispInfo = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    if (audioCmdParams.param1 == 0)  //get data
    {
        pGainForDispInfo = malloc(MAX_BUFSIZE_RECV);
        if (pGainForDispInfo == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pGainForDispInfo, responseStr, strlen(responseStr)));

		if(audioCmdParams.param2 < 0 || audioCmdParams.param2 > MAX_BLOCK_NUM)
		{
			ALOGD("block number:%d is invailed", audioCmdParams.param2);
			ret = ACHFailed;
		}
		else
		{
			ALOGD("pGainForDispInfo is 0x%x", (int)pGainForDispInfo);
			ret = pAudioHandle->getGainInfoForDisp((char *)pGainForDispInfo + strLen, &dataLen, audioCmdParams.param2);
		}
		pGainForDispInfo[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
			for(int i=0; i< 96; )
			{
				ALOGD("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",*((char *)pGainForDispInfo+i), *((char *)pGainForDispInfo+i+1), *((char *)pGainForDispInfo+i+2), *((char *)pGainForDispInfo+i+3), *((char *)pGainForDispInfo+i+4), *((char *)pGainForDispInfo+i+5), *((char *)pGainForDispInfo+i+6), *((char *)pGainForDispInfo+i+7));
				i += 8;
			}
            sendResponseToATCI((char *)pGainForDispInfo, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pGainForDispInfo);
    }
    else
    {
		ALOGE("audioCmdParams.param1:%d is invailed", audioCmdParams.param1);

        strncat(responseStr, kRespFail, kRespFailLen);
        sendResponseToATCI(responseStr, strlen(responseStr));
    }

}

static void DoCustGainParam(AudioCmdParam &audioCmdParams, char *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EGAIN=0,0");
    void *pGainParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

	if (audioCmdParams.param1 == 0)
	{
		pGainParam = malloc(MAX_BUFSIZE_RECV);
        if (pGainParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
		strncpy((char *)pGainParam, responseStr, strlen(responseStr)));

		if(audioCmdParams.param2 < 0 || audioCmdParams.param2 > MAX_BLOCK_NUM)
		{
			ALOGD("block number:%d is invailed", audioCmdParams.param2);
			ret = ACHFailed;
		}
		else
		{
			ret = pAudioHandle->ULCustGainParamFromNV((char *)pGainParam + strLen, &dataLen, audioCmdParams.param2);
		}
		pGainParam[strLen + dataLen] = '\0';

		if (ret == ACHSucceeded)
		{
			sendResponseToATCI((char *)pGainParam, strLen + dataLen);
		}
		else
		{
			strncat(responseStr, kRespFail, kRespFailLen);
			sendResponseToATCI(responseStr, strlen(responseStr));
		}

		free(pGainParam);
	}
	else if(audioCmdParams.param1 == 1)
	{
		pGainParam = pParam + strLen;

		if(audioCmdParams.param2 < 0 || audioCmdParams.param2 > MAX_BLOCK_NUM)
		{
			ALOGD("block number:%d is invailed", audioCmdParams.param2);
			ret = ACHFailed;
		}
		else
		{
			ret = pAudioHandle->DLCustGainParamToNV((char *)pGainParam, audioCmdParams.param2);
		}

		if (ret == ACHSucceeded)
		{
			strncat(responseStr, kRespOK, kRespOKLen);
			sendResponseToATCI(responseStr, strlen(responseStr));
		}
		else
		{
			strncat(responseStr, kRespFail, kRespFailLen);
			sendResponseToATCI(responseStr, strlen(responseStr));
		}

	}
	else
	{
		ALOGE("audioCmdParams.param1:%d is invailed", audioCmdParams.param1);

        strncat(responseStr, kRespFail, kRespFailLen);
        sendResponseToATCI(responseStr, strlen(responseStr));
	}
}

static void DoCustBTGAINParam(AudioCmdParam &audioCmdParams, char *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EGAIN_NREC=0");
    char *pSpeechParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];

    if (audioCmdParams.param1 == 0)
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char*)malloc(MAX_BUFSIZE_RECV);
        if (pSpeechParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pSpeechParam, responseStr, strlen(responseStr));

		ret = pAudioHandle->ULCustBtNrecParamFromNV(pSpeechParam + strLen, &dataLen);
		pSpeechParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pSpeechParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pSpeechParam);
    }
    else
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = pParam + strLen;

		ret = pAudioHandle->DLCustBtNrecParamToNV(pSpeechParam);
        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}

static void DoGetBtNrecInfoForDisp(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EDISPITEM_NREC=0");
    void *pGainForDispInfo = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    if (audioCmdParams.param1 == 0)  //get data
    {
        pGainForDispInfo = malloc(MAX_BUFSIZE_RECV);
        if (pGainForDispInfo == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pGainForDispInfo, responseStr, strlen(responseStr));

		ALOGD("pGainForDispInfo is 0x%x", (int)pGainForDispInfo);
		ret = pAudioHandle->getBtNrecInfoForDisp((char *)pGainForDispInfo + strLen, &dataLen);
		pGainForDispInfo[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pGainForDispInfo, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pGainForDispInfo);
    }
    else
    {
		ALOGE("audioCmdParams.param1:%d is invailed", audioCmdParams.param1);

        strncat(responseStr, kRespFail, kRespFailLen);
        sendResponseToATCI(responseStr, strlen(responseStr));
    }
}
#endif

static void DoHDRecording(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int strLen = sizeof("MM+ERECORD=0");

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    if (pAudioHandle->HDRecording(audioCmdParams.param1, (char *)pParam + strLen) == ACHSucceeded)
    {
        strncat(responseStr, kRespOK, kRespOKLen);
    }
    else
    {
        strncat(responseStr, kRespFail, kRespFailLen);
    }

    sendResponseToATCI(responseStr, strlen(responseStr));
}

static void DoGetPhoneSupportInfo(AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    unsigned int supportInfo = 0;
    char paramsKeyPaires[MAX_CMD_LEN];

    if (pAudioHandle != NULL)
    {
        ret = pAudioHandle->getPhoneSupportInfo(&supportInfo);
    }

    if (ret == ACHSucceeded)
    {
        snprintf(paramsKeyPaires, MAX_CMD_LEN, "MM+GPSI=%d\r\n", supportInfo);
        sendResponseToATCI(paramsKeyPaires, strlen(paramsKeyPaires));
    }
    else
    {
        sendResponseToATCI("MM+GPSI=Failed\r\n", strlen("MM+GPSI=Failed\r\n"));
    }
}
static void DoCustHFPParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EHFP=0");
    char *pSpeechParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];

    if (audioCmdParams.param1 == 0)
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pSpeechParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pSpeechParam, responseStr, strlen(responseStr));

		ret = pAudioHandle->ULCustHFPParamFromNV(pSpeechParam + strLen, &dataLen);
		pSpeechParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pSpeechParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pSpeechParam);
    }
    else
    {
        strncpy(responseStr, (char *)pParam, strLen - 1);
        responseStr[strLen - 1] = '\0';

        pSpeechParam = (char *)pParam + strLen;

		ret = pAudioHandle->DLCustHFPParamToNV(pSpeechParam);
        if (ret == ACHSucceeded)
        {
            strncat(responseStr, kRespOK, kRespOKLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}

static void DoCustVOIPParam(AudioCmdParam &audioCmdParams, void *pParam, AudioCmdHandler *pAudioHandle)
{
    int ret = ACHFailed;
    int dataLen = 0;
    int strLen = sizeof("MM+EVOIP=0");
    char *pVOIPParam = NULL;

    if (pAudioHandle == NULL || pParam == NULL)
    {
        return;
    }

    // extract command string
    char responseStr[32];
    strncpy(responseStr, (char *)pParam, strLen - 1);
    responseStr[strLen - 1] = '\0';

    if (audioCmdParams.param1 == 0)
    {
        pVOIPParam = (char *)malloc(MAX_BUFSIZE_RECV);
        if (pVOIPParam == NULL) {
            ALOGD("malloc failed");
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
            return;
        }
        strncpy((char *)pVOIPParam, responseStr, strlen(responseStr));

        ret = pAudioHandle->ULCustVOIPParamFromNV(pVOIPParam + strLen, &dataLen);
		pVOIPParam[strLen + dataLen] = '\0';

        if (ret == ACHSucceeded)
        {
            sendResponseToATCI((char *)pVOIPParam, strLen + dataLen);
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }

        free(pVOIPParam);
    }
    else
    {
        pVOIPParam = (char *)pParam + strLen;
        ret = pAudioHandle->DLCustVOIPParamToNV(pVOIPParam);

        if (ret == ACHSucceeded)
        {
            strncat(responseStr, " OK", 3);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
        else
        {
            strncat(responseStr, kRespFail, kRespFailLen);
            sendResponseToATCI(responseStr, strlen(responseStr));
        }
    }
}
//--->

//parse command received from ATCI
#define PARA_NUM 5
#define PARA_STRLEN 100
static AudioCmdType parseCmd(char *pRecvData, AudioCmdParam &mAudioCmdParams)
{
    char tempData[MAX_CMD_LEN];
    memset (tempData, 0, MAX_CMD_LEN);
    if (strlen(pRecvData) > (MAX_CMD_LEN - 1))
    {
        return UNKNOWN_CMD;
    }

    strncpy(tempData, pRecvData, strlen(pRecvData));
    char *pTempData = tempData;
    char *pRecvCmd = NULL;
    char *pRecvParams = NULL;
    char *token = NULL;
    int mParams[5];
    char mStrParams[PARA_NUM][PARA_STRLEN];
    AudioCmdType mReturnVal = UNKNOWN_CMD;

    ALOGD("Audio Cmd Handler start to parse command in, received data %s, len %d", pRecvData, strlen(pRecvData));

    memset(&mAudioCmdParams, 0, sizeof(mAudioCmdParams));
    memset(mParams, 0, sizeof(mParams));

    pRecvCmd = strsep(&pTempData, "=");
    if (NULL == pRecvCmd)
    {
        pRecvCmd = pTempData;
    }
    else if (strlen(pRecvData) == strlen(pRecvCmd) + 1)
    {
        pRecvParams = NULL;
    }
    else
    {
        pRecvParams = pTempData;
    }

    if (pRecvParams)
    {
        int i = 0;
        do
        {
            token = strsep(&pRecvParams, ",");

            if (i<PARA_NUM)
            {
                snprintf(mStrParams[i], PARA_STRLEN, "%s", token);
            }
            else
            {
                ALOGD("Audio Cmd Handler===> too much parameter");
            }

            if (!token)
            {
                mParams[i] = atoi(pRecvParams);
            }
            else
            {
                mParams[i] = atoi(token);
            }
            //          ALOGD("The parameters is mParams[%d]=%d, token=%d, pRecvParams=%d", i, mParams[i], token, pRecvParams);
            i++;
        }
        while (pRecvParams && i < 5);

        mAudioCmdParams.param1 = mParams[0];
        mAudioCmdParams.param2 = mParams[1];
        mAudioCmdParams.param3 = mParams[2];
        mAudioCmdParams.param4 = mParams[3];
        mAudioCmdParams.param5 = mParams[4];
        mAudioCmdParams.recvDataLen = strlen(pRecvData);
    }

    if (0 == strcmp(pRecvCmd, "AUD+RDPRM"))
    {
        mReturnVal = RDPRM_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "AUD+VOL"))
    {
        mParams[0] = GetStreamTypeIndex(mStrParams[0]);
        mParams[1] = GetDeviceTypeIndex(mStrParams[1]);
        mReturnVal = RT_VOL_CMD;
        mAudioCmdParams.param1 = mParams[0];
        mAudioCmdParams.param2 = mParams[1];
    }
    else if (0 == strcmp(pRecvCmd, "MM+VOL"))
    {
        mReturnVal = EVOL_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EADP"))
    {
        mReturnVal = EADP_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EAPS"))
    {
        mReturnVal = EAPS_CMD;
    }
	else if(0 == strcmp(pRecvCmd, "MM+EMAGI"))
	{
		mReturnVal = EMAGI_CMD;
	}
	else if(0 == strcmp(pRecvCmd, "MM+EHAC"))
	{
		mReturnVal = EHAC_CMD;
	}
	else if(0 == strcmp(pRecvCmd, "MM+ESLBK"))
	{
		mReturnVal = ELPBK_CMD;
	}
	else if(0 == strcmp(pRecvCmd, "MM+EDISPITEM_NREC"))
	{
		mReturnVal = EDISPBTNREC_CMD;
	}
	else if(0 == strcmp(pRecvCmd, "MM+EGAIN_NREC"))
	{
		mReturnVal = EBTGAIN_CMD;
	}
    else if (0 == strcmp(pRecvCmd, "MM+EACF"))
    {
        mReturnVal = EACF_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EHCF"))
    {
        mReturnVal = EHCF_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EDUALMIC"))
    {
        mReturnVal = EDUALMIC_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EDMNRPLYREC"))
    {
        mReturnVal = EDMNRPLYREC_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EDMNRGSET"))
    {
        mReturnVal = EDUALMICGAIN_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EHDREC"))
    {
        mReturnVal = EHDREC_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+ERECORD"))
    {
        mReturnVal = ERECORD_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+GPSI"))
    {
        mReturnVal = GPSI_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EAUDT"))
    {
        mReturnVal = EAUDT_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+GCVI"))
    {
        mReturnVal = GCVI_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EVOIP"))
    {
        mReturnVal = EVOIP_CMD;
    }
    else if (0 == strcmp(pRecvCmd, "MM+EDISPITEM"))
    {
        mReturnVal = EDISPITEM_CMD;
    }
	else if(0 == strcmp(pRecvCmd, "MM+EHFP"))
	{
		mReturnVal = EHFP_CMD;
	}
	    else if (0 == strcmp(pRecvCmd, "MM+EGAIN"))
    {
        mReturnVal = EGAIN_CMD;
    }
    else
    {
        mReturnVal = UNKNOWN_CMD;
        ALOGE("Unknown command %s", pRecvCmd);
    }

    ALOGD("Parse command success, the command is %s", pRecvCmd);
    return mReturnVal;
}

void sendResponseToATCI(const char *pSendData, int length)
{
    int mSendLen = 0;
    ALOGD("The data that Audio Cmd Handler sent to ATCI is %s, data length is %d", pSendData, length);

    if (s_fdAudioCmdHdlSrv_command >= 0)
    {
        mSendLen = send(s_fdAudioCmdHdlSrv_command, pSendData, length, 0);
        if (mSendLen != length)
        {
            ALOGE("Lose data when Audio Cmd Handler send to ATCI. errno = %d", errno);
        }
    }
    else
    {
        ALOGE("Fail to send response to ATCI, the connection is disconnect");
    }
}

static void *audioCmdHandleLoop(void *pParam)
{
    unused(pParam);
    int ret = 0;
    struct sockaddr_in ATCI_addr;
    int ATCI_len = sizeof(ATCI_addr);
    fd_set rfds;
    AudioCmdType mCmdType = UNKNOWN_CMD;
    AudioCmdHandler *pAudioCmdHandler = NULL;

    ALOGD("Audio Cmd Handler data processing loop in");
#ifdef DISABLE_AUDIO_DAEMON_IN_USER_LOAD
	char buildType[PROPERTY_VALUE_MAX] = {0};
	property_get(BUILD_TYPE_PROP, buildType, "user");
	ALOGD("Build type :%s",buildType);			
	if(IS_BUILDTYPE(buildType,BUILD_TYPE_PROP_USER)) 
	{
		exit(-1);
	}
#endif
    while (1)
    {
        if (s_fdAudioCmdHdlSrv_command < 0)
        {
            FD_ZERO(&rfds);
            FD_SET(s_fdAudioCmdHdlSrv_listen, &rfds);
            ret = select(s_fdAudioCmdHdlSrv_listen + 1, &rfds, NULL, NULL, NULL);

            if (ret < 0)
            {
                if (errno == EINTR) { continue; }
                ALOGE("Fail to select. error (%d)", errno);
                exit(-1);
            }

            if (FD_ISSET(s_fdAudioCmdHdlSrv_listen, &rfds))
            {
                s_fdAudioCmdHdlSrv_command = accept(s_fdAudioCmdHdlSrv_listen, (sockaddr *)&ATCI_addr, &ATCI_len);

                if (s_fdAudioCmdHdlSrv_command < 0)
                {
                    ALOGE("Error on accept(s_fdAudioCmdHdlSrv_command) errno:%d", errno);
                    exit(-1);
                }
                ALOGD("Connect to ATCI server success.");

                ret = fcntl(s_fdAudioCmdHdlSrv_command, F_SETFL, O_NONBLOCK);
                if (ret < 0)
                {
                    ALOGE("Fail to set audio-atci server socket O_NONBLOCK. errno: %d", errno);
                }

                if (pAudioCmdHandler == NULL)
                {
                    pAudioCmdHandler = new AudioCmdHandler();
                }
            }
        }
        else
        {
            FD_ZERO(&rfds);
            FD_SET(s_fdAudioCmdHdlSrv_command, &rfds);
            ret = select(s_fdAudioCmdHdlSrv_command + 1, &rfds, NULL, NULL, NULL);

            if (ret < 0)
            {
                if (errno == EINTR) { continue; }
                ALOGE("Fail to select. error (%d)", errno);

                if (pAudioCmdHandler != NULL)
                {
                    delete pAudioCmdHandler;
                    pAudioCmdHandler = NULL;
                }
                close(s_fdAudioCmdHdlSrv_command);
                s_fdAudioCmdHdlSrv_command = -1;
                exit(-1);
            }

            if (FD_ISSET(s_fdAudioCmdHdlSrv_command, &rfds))
            {
                int recvLen = 0;
                AudioCmdParam mAduioCmdParams;
                char pRecvData[MAX_BUFSIZE_RECV];
                char paramsKeyPaires[MAX_CMD_LEN];

                memset(pRecvData, 0, MAX_BUFSIZE_RECV);
                recvLen = recv(s_fdAudioCmdHdlSrv_command, pRecvData, MAX_BUFSIZE_RECV, 0);
                ALOGD("Receive data from audio-atci client socket. length = %d", recvLen);
                if (recvLen <= 0)
                {
                    ALOGE("Fail to receive data from audio-atci client socket. errno = %d", errno);
                    continue;
                }
                else
                {
                    mCmdType = parseCmd(pRecvData, mAduioCmdParams);
                }

                switch (mCmdType)
                {
                    case RDPRM_CMD:
                        DoSetRecorderParam(mAduioCmdParams, pAudioCmdHandler);
                        break;
                    case RT_VOL_CMD: //Maybe obsolete after Android P, use EVOL_CMD instead
                        if(( mAduioCmdParams.param1 != -1) && ( mAduioCmdParams.param2 != -1))
                        {
                            ALOGD("RT_VOL_CMD streamtype %d device index %d volume %d",mAduioCmdParams.param1,mAduioCmdParams.param2,mAduioCmdParams.param3);
                            StartDeviceIdQuery();
                            int DeviceId = QueryDeviceId(mAduioCmdParams.param2);
                            while ( DeviceId != AUDIO_DEVICE_NONE) {
                                ALOGD("RT_VOL_CMD streamtype %d device %d volume %d",mAduioCmdParams.param1,DeviceId,mAduioCmdParams.param3);
                                pAudioCmdHandler->RealTimeSetVolume(mAduioCmdParams.param1, mAduioCmdParams.param3, DeviceId);
                                DeviceId = QueryDeviceId(mAduioCmdParams.param2);
                            }
                            sendResponseToATCI("AUD+VOL=Succeeded", strlen("AUD+VOL=Succeeded"));
                        }
                        else
                        {
                            sendResponseToATCI("AUD+VOL=failed", strlen("AUD+VOL=failed"));
                        }
                        break;
                    case EVOL_CMD:
                        DoGetVolume(pAudioCmdHandler);
                        break;
                    case EAPS_CMD:
                        DoCustSPHParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case EMAGI_CMD:
                        DoCustSPHMagiParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;				
					case EHAC_CMD:
						DoCustSPHHACParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
						break;	
					case ELPBK_CMD:
						DoCustSPHLPBKParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
						break;
                    case EADP_CMD:
                        DoCustSPHVolumeParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case EACF_CMD:
//                        		mAduioCmdParams.param2 = 0;
                        DoCustACFHCFParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case EHCF_CMD:
                        mAduioCmdParams.param2 = 1;
                        DoCustACFHCFParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case EDUALMIC_CMD:
                        DoCustDUALMICParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case EDMNRPLYREC_CMD:
                        DoCustDualMicNR(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case EDUALMICGAIN_CMD:
                        DoCustDMNRGainSetting(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case EHDREC_CMD:
                        DoCustHDRecParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case ERECORD_CMD:
                        DoHDRecording(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case GPSI_CMD:
                        DoGetPhoneSupportInfo(pAudioCmdHandler);
                        break;
                    case EAUDT_CMD:
                        DoAudioTaste(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case GCVI_CMD:
                        char value[PROPERTY_VALUE_MAX];
                        property_get("ro.vendor.mediatek.platform", value, "0");

#if !defined(MTK_AUDIO_GAIN_TABLE) || (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2)
                        snprintf(paramsKeyPaires, MAX_CMD_LEN, "+GCVI:\"%s\" OK\r\n", value);
#else
                        snprintf(paramsKeyPaires, MAX_CMD_LEN, "+GCVI:\"TC1:%s\" OK\r\n", value);
#endif
                        ALOGD("Audio Cmd Handler get Chip id:%s", paramsKeyPaires);
                        sendResponseToATCI(paramsKeyPaires, strlen(paramsKeyPaires));
                        break;
                    case EVOIP_CMD:
                        DoCustVOIPParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
                        break;
                    case UNKNOWN_CMD:
                        sendResponseToATCI("AUD+=Unknown", strlen("AUD+=Unknown"));
                        break;
					case EHFP_CMD:
						DoCustHFPParam(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
						break;
#ifdef MTK_AUDIO_GAIN_TABLE
					case EDISPITEM_CMD:
						DoGetGainInfoForDisp(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
						break;
					case EGAIN_CMD:
						DoCustGainParam(mAduioCmdParams, pRecvData, pAudioCmdHandler);
						break;
					case EDISPBTNREC_CMD:
						DoGetBtNrecInfoForDisp(mAduioCmdParams, (void *)pRecvData, pAudioCmdHandler);
						break;
					case EBTGAIN_CMD:
						DoCustBTGAINParam(mAduioCmdParams, pRecvData, pAudioCmdHandler);
						break;
#endif
                    default:
                        ALOGD("Unhandled case in switch (mCmdType)");
                        break;
                }
            }
        }
    }
    ALOGD("Audio Cmd Handler data processing loop out");
}

int main(int argc, char *argv[])
{
    unused(argc);
    //unused(argv);
    int ret = 0;
    pthread_t s_tid_audio;

    char buf[256];
    getcwd(buf, 256);
    ALOGD("AudioCmdHandlerService main() in %s", argv[0]);

    // set up thread-pool

    ProcessState::initWithDriver("/dev/vndbinder");
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    s_fdAudioCmdHdlSrv_listen = android_get_control_socket(SOCKET_NAME_AUDIO);
    ALOGD("Get socket '%d': %s", s_fdAudioCmdHdlSrv_listen, strerror(errno));
    if (s_fdAudioCmdHdlSrv_listen < 0)
    {
        ALOGE("Failed to get socket '" SOCKET_NAME_AUDIO "'");
        exit(-1);
    }

    ret = listen(s_fdAudioCmdHdlSrv_listen, 4);
    if (ret < 0)
    {
        ALOGE("Failed to listen on control socket '%d': %s", s_fdAudioCmdHdlSrv_listen, strerror(errno));
        exit(-1);
    }

    ret = fcntl(s_fdAudioCmdHdlSrv_listen, F_SETFL, O_NONBLOCK);
    if (ret < 0)
    {
        ALOGE("Fail to set audio-atci server socket O_NONBLOCK. errno: %d", errno);
    }

    ret = pthread_create(&s_tid_audio, NULL, audioCmdHandleLoop, NULL);
    if (ret != 0)
    {
        ALOGE("Fail to create audio-handler thread errno:%d", errno);
        exit(-1);
    }

    pthread_join(s_tid_audio, NULL);
    return 0;
}

