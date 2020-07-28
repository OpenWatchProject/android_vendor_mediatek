/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "AppDSDN"

#include "AppDSDN.h"
#define DSDN_VERSION "V1.0.0"

/*******************************************************************************
*
********************************************************************************/
MTKDSDN*
AppDSDN::
getInstance()
{
    MTKDSDN *mAppDSDN;
    LOGD("getInstance \n");
    LOGD("[%s] VERSION %s \n", LOG_TAG, DSDN_VERSION);

    mAppDSDN = new AppDSDN();

    return mAppDSDN;
}

/*******************************************************************************
*
********************************************************************************/
void
AppDSDN::
destroyInstance(MTKDSDN* obj)
{
    LOGD("destroyInstance\n");

    if (obj != NULL) {
       delete obj;
    }
}

/*******************************************************************************
*
********************************************************************************/
AppDSDN::AppDSDN()
{
    LOGD("constructor\n");

    pDSDNCore = new DSDNCore();
}

/*******************************************************************************
*
********************************************************************************/
AppDSDN::~AppDSDN()
{
    LOGD("destructor\n");

    if(pDSDNCore!=NULL)
    {
        delete pDSDNCore;
    }
}

MRESULT AppDSDN::Init(void* InitInData, void* InitOutData)
{
    LOGD("Init \n");

    MRESULT ret = S_DSDN_OK;

    DSDNInitInfo* pInitInfo;
    DSDNTuningInfo* pTuningInfo;
    pInitInfo = (DSDNInitInfo*)InitInData;
    pTuningInfo = pInitInfo->pTuningInfo;

    if(pInitInfo->dsdnMode >1)
    {
        LOGD("Invalid parameter. dsdnMode(%d)\n", pInitInfo->dsdnMode);
        ret = E_DSDN_INVALID_PARAM;
        return ret;
    }

    initInfo.dsdnMode = pInitInfo->dsdnMode;

    if(pInitInfo->maxWidth ==0 || pInitInfo->maxHeight ==0)
    {
        LOGD("Invalid parameter. maxWidth(%d), maxHeight(%d)\n", pInitInfo->maxWidth, pInitInfo->maxHeight);
        ret = E_DSDN_INVALID_PARAM;\
        return ret;
    }

    initInfo.maxWidth = pInitInfo->maxWidth;
    initInfo.maxHeight = pInitInfo->maxHeight;

    if(pTuningInfo->NumOfParam < 0 || pTuningInfo->NumOfParam > DSDN_MAX_TUNING || pTuningInfo->params == NULL)
    {
        LOGD("Invalid tuning parameter NumOfParam(%d)\n", pTuningInfo->NumOfParam);
        tuningInfo.NumOfParam = 0;
        tuningInfo.params = NULL;
    }
    else
    {
        tuningInfo.NumOfParam = pTuningInfo->NumOfParam;
        tuningInfo.params = pTuningInfo->params;
    }
    LOGD("NumOfParam (%d) params(0x%X)\n", tuningInfo.NumOfParam, tuningInfo.params);

    initInfo.pTuningInfo = &tuningInfo;

    LOGD("DSDN mode (%d)\n", pInitInfo->dsdnMode);
    LOGD("maxWidth(%d), maxHeight(%d)\n", pInitInfo->maxWidth, pInitInfo->maxHeight);

    ret = pDSDNCore->initialize(&initInfo);

    LOGD("Init End\n");

    return ret;
}

MRESULT AppDSDN::Main(void)
{
    if(pDSDNCore->mLogLevel)
    {
        LOGD("Main Start\n");
    }
    
    MRESULT ret = S_DSDN_OK;
    ret = pDSDNCore->doDSDNMain();

    if(pDSDNCore->mLogLevel)
    {
        LOGD("Main End\n");
    }

    return ret;
}

MRESULT AppDSDN::Reset(void)
{
    LOGD("Reset\n");
    pDSDNCore->reset();
    return S_DSDN_OK;
}

MRESULT AppDSDN::FeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut)
{
    if(pDSDNCore->mLogLevel)
    {
        LOGD("FeatureCtrl  FeatureID(%d) \n", FeatureID);
    }
    
    MRESULT ret = S_DSDN_OK;
    MUINT32* bufferSize = (MUINT32*)pParaOut;
    DSDNInitInfo* initInfo = (DSDNInitInfo*)pParaIn;
    DSDNProcInfo* procInfo = (DSDNProcInfo*)pParaIn;

    switch(FeatureID)
    {
        case DSDN_FEATURE_GET_WORKBUF_SIZE:
            *bufferSize = pDSDNCore->getBufferSizeNeeded();
        break;

        case DSDN_FEATURE_SET_WORKBUF_ADDR:
            pDSDNCore->setupBuffer(initInfo);
        break;

        case DSDN_FEATURE_SET_PROC_INFO:
            pDSDNCore->setProcInfo(procInfo);
        break;

        default:
            LOGD("FeatureCtrl.  E_DSDN_WRONG_CMD_ID.\n");
            ret = E_DSDN_WRONG_CMD_ID;
            break;
    };

    if(pDSDNCore->mLogLevel)
    {
        LOGD("FeatureCtrl  FeatureID(%d) End\n", FeatureID);
    }

    return S_DSDN_OK;
}

