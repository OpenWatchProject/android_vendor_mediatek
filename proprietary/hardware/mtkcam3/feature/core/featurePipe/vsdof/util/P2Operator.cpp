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


// Standard C header file
#include <cstring>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "P2Operator.h"

#define PIPE_MODULE_TAG "vsdof_util"
#define PIPE_CLASS_TAG "P2Operator"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

using namespace std;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

/*******************************************************************************
* Global Define
*******************************************************************************/
using AutoObject = std::unique_ptr<const char, std::function<void(const char*)>>;
//
#define SCOPE_LOGGER auto __scope_logger__ = create_scope_logger(__FUNCTION__)
static AutoObject create_scope_logger(const char* functionName)
{
    MY_LOGD("[%s] + ", functionName);
    return AutoObject(functionName, [](const char* p)
        {
            MY_LOGD("[%s] -", p);
        });
}

#define SCOPE_TIMER(extInfo) auto __scope_timer__ = create_scope_timer(__FUNCTION__, extInfo)
static AutoObject create_scope_timer(const char* funcName, const char* extInfo)
{
    static const char* split = "-";

    const MINT32 length = ::strlen(funcName) + ::strlen(split) + ::strlen(extInfo) + 1;
    char* pBuf = new char[length];
    ::strncpy(pBuf, funcName, length );
    ::strncat(pBuf, split, length - ::strlen(funcName));
    ::strncat(pBuf, extInfo, length - ::strlen(funcName) - ::strlen(split));

    MY_LOGD("[%s] + ", pBuf);
    return AutoObject(pBuf, [](const char* p)
        {
            MY_LOGD("[%s] -", p);
            delete[] p;
        });
}

/*******************************************************************************
* External Function
********************************************************************************/


/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
P2Operator::P2Operator(
    const char* creatorName,
    MINT32 openId
    )
  : mCreatorName(creatorName)
  , miOpenId(openId)
{
    MY_LOGD("OpenId(%d) CreatorName(%s)", miOpenId, mCreatorName);

    SCOPE_TIMER("create normal stream");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miOpenId);

    mpINormalStream->init(LOG_TAG);
}

P2Operator::~P2Operator()
{
    SCOPE_LOGGER;
}

MVOID
P2Operator::
onLastStrongRef( const void* /*id*/)
{
    SCOPE_LOGGER;

    if(mpINormalStream != nullptr){
        mpINormalStream->uninit(LOG_TAG);
        mpINormalStream->destroyInstance();
        mpINormalStream = nullptr;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
status_t
P2Operator::
enque(QParams enqueParam, const char* userName)
{
    SCOPE_LOGGER;
    Mutex::Autolock _l(mLock);

    if(mpINormalStream == nullptr){
        MY_LOGE("mpINormalStream == nullptr");
        return UNKNOWN_ERROR;
    }

    if(enqueParam.mpfnCallback == nullptr || enqueParam.mpfnEnQFailCallback == nullptr){
        MY_LOGE("P2Operator only support non-blocking p2 operations! (%p,%p)",
            enqueParam.mpfnCallback,
            enqueParam.mpfnEnQFailCallback
        );
        return UNKNOWN_ERROR;
    }

    MY_LOGD("normalStream::enque [%s] +", userName);

    if(!mpINormalStream->enque(enqueParam)){
        MY_LOGE("mpINormalStream enque failed!");
        return MFALSE;
    }

    MY_LOGD("normalStream::enque [%s] -", userName);

    return OK;
}

status_t
P2Operator::
release()
{
    // remove if not needed
    return OK;
}
