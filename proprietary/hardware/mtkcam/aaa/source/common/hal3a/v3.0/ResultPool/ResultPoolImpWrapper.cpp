/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "ResultPoolImpWrapper"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>

//For Metadata
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "aaa_utils.h"


#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

#include <mutex>

//tuning utils
#include <SttBufQ.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/sys/IFileCache.h>

#include <sys/stat.h>


#include "ResultPoolImpWrapper.h"

using namespace NS3Av3;
using namespace NSCam::TuningUtils;


#define GET_PROP(prop, dft, val)\
{\
   val = property_get_int32(prop,dft);\
}

#define HALISP_LOG_SET_P1       (1<<0)
#define HALISP_LOG_SET_P2       (2<<0)
#define HALISP_LOG_SETPARAM_P2  (3<<0)
#define HALISP_LOG_GET_P1       (4<<0)
#define HALISP_LOG_GET_P2       (5<<0)
#define HALISP_LOG_CONVERT_P2   (6<<0)


/*******************************************************************************
* implementations
********************************************************************************/

ResultPoolImpWrapper::
ResultPoolImpWrapper(MINT32 const i4SensorDev)
    : ResultPoolImp(i4SensorDev)
    , m_i4SensorDev(i4SensorDev)
{
    CAM_LOGD("[%s] sensorDev(0x%04x)", __FUNCTION__, m_i4SensorDev);
}

ResultPoolImpWrapper*
ResultPoolImpWrapper::
getInstance(MINT32 const i4SensorDev)
{
    GET_PROP("vendor.debug.resultpool.log", 0, m_u4LogEn);
    switch (i4SensorDev) {
        case ESensorDev_Main: {

            static ResultPoolImpWrapper _singleton(ESensorDev_Main);
            return &_singleton;
        }
        break;
        case ESensorDev_Sub: {

            static ResultPoolImpWrapper _singleton(ESensorDev_Sub);
            return &_singleton;
        }
        break;
        case ESensorDev_MainSecond: {

            static ResultPoolImpWrapper _singleton(ESensorDev_MainSecond);
            return &_singleton;
        }
        break;
        case ESensorDev_SubSecond: {

            static ResultPoolImpWrapper _singleton(ESensorDev_SubSecond);
            return &_singleton;
        }
        break;
        case ESensorDev_MainThird: {

            static ResultPoolImpWrapper _singleton(ESensorDev_MainThird);
            return &_singleton;
        }
        break;
        default:
            CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDev);
            return MNULL;
    }
}

MINT32
ResultPoolImpWrapper::
getAllHQCResultWithType(MUINT32 frmId, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, AllResult_T** pAllResult, const char* pFunctionName)
{
    CAM_LOGD("[%s] get R[%d]", __FUNCTION__, frmId);

    *pAllResult = this->getResultBySttNum(frmId, pFunctionName);
    return this->isValidateBySttNum(frmId, eModule);
}


MINT32
ResultPoolImpWrapper::
getOldestAllResultWithType(const E_PARTIAL_RESULT_OF_MODULE_T& eModule, AllResult_T** pAllResult, const char* pFunctionName)
{
    // ResultPool - Get History
    MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
    this->getHistory(rHistoryReqMagic);

    CAM_LOGD("[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d), current result will be obtained(%d).", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2], rHistoryReqMagic[2]);

    *pAllResult = this->getResultByReqNum(rHistoryReqMagic[2], pFunctionName);//get the last request EX: req/stt:4/1

    return this->isValidateByReqNum(rHistoryReqMagic[2], eModule);
}

MINT32
ResultPoolImpWrapper::
getAllResultWithType(MUINT32 frmId, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, AllResult_T** pAllResult, const char* pFunctionName)
{
    CAM_LOGD("[%s] Get Result with ReqMagic(%d)", __FUNCTION__, frmId);

    *pAllResult = this->getResultByReqNum(frmId, pFunctionName);//req/stt:2/X

    return this->isValidateByReqNum(frmId, eModule);
}

MVOID
ResultPoolImpWrapper::
getPreviousResultWithType(const E_PARTIAL_RESULT_OF_MODULE_T& eModule, ISPResultToMeta_T** pISPResult)
{
    // ResultPool - Get History
    MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
    this->getHistory(rHistoryReqMagic);

    *pISPResult = (ISPResultToMeta_T*)this->getResult(rHistoryReqMagic[1], eModule, __FUNCTION__);

    CAM_LOGD("[%s], Get ISPResult(%p) at MagicNum(%d) Instead", __FUNCTION__, *pISPResult, rHistoryReqMagic[1]);
}

