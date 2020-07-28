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

#include "P2CamContext.h"

#define PIPE_CLASS_TAG "P2CamContext"
#define PIPE_TRACE TRACE_P2_CAM_CONTEXT
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);


using namespace android;
using namespace NSCam::NR3D;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


const char* P2CamContext::MODULE_NAME = "FeaturePipe_P2";

Mutex P2CamContext::sMutex;
sp<P2CamContext> P2CamContext::spInstance[P2CamContext::SENSOR_INDEX_MAX];


P2CamContext::P2CamContext(MUINT32 sensorIndex)
    : mRefCount(0)
    , mSensorIndex(sensorIndex)
    , mIsInited(MFALSE)
    , mp3dnr(NULL)
    , mp3A(NULL)
    , mpISP(NULL)
{
}


P2CamContext::~P2CamContext()
{
    MY_LOGD("P2CamContext[%d]: destructor is called", mSensorIndex);
    uninit();
}


sp<P2CamContext> P2CamContext::createInstance(
    MUINT32 sensorIndex,
    const StreamingFeaturePipeUsage &pipeUsage)
{
    if (sensorIndex >= SENSOR_INDEX_MAX)
        return NULL;

    Mutex::Autolock lock(sMutex);

    sp<P2CamContext> inst = spInstance[sensorIndex];

    if (inst == NULL)
    {
        inst = new P2CamContext(sensorIndex);
        inst->init(pipeUsage);
        spInstance[sensorIndex] = inst;
    }

    inst->mRefCount++;
    MY_LOGD("P2CamContext[%d]: mRefCount increased: %d", sensorIndex, inst->mRefCount);

    return inst;
}


void P2CamContext::destroyInstance(MUINT32 sensorIndex)
{
    if (sensorIndex >= SENSOR_INDEX_MAX)
        return;

    Mutex::Autolock lock(sMutex);

    sp<P2CamContext> inst = spInstance[sensorIndex];

    if (inst != NULL)
    {
        // Do not call uninit() here, because the instance may be still hold
        // by some running thread
        inst->mRefCount--;
        MY_LOGD("P2CamContext[%d]: mRefCount decreased: %d", sensorIndex, inst->mRefCount);
        if (inst->mRefCount <= 0)
            spInstance[sensorIndex] = NULL;
    }
}


sp<P2CamContext> P2CamContext::getInstance(MUINT32 sensorIndex)
{
    if (sensorIndex >= SENSOR_INDEX_MAX)
        return NULL;

    // NOTE: sp<> is not atomic, should be kept in critical section
    Mutex::Autolock lock(sMutex);

    sp<P2CamContext> inst = spInstance[sensorIndex];
    if (inst == NULL)
    {
        CAM_LOGF("P2CamContext[%d] was not created!", sensorIndex);
    }

    return inst;
}


void P2CamContext::init(const StreamingFeaturePipeUsage &pipeUsage)
{
    TRACE_FUNC_ENTER();

    if (!mIsInited)
    {
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "CamContext::init3A_ISP");
        if( mp3A == NULL && SUPPORT_3A_HAL )
        {
            mp3A = MAKE_Hal3A(mSensorIndex, PIPE_CLASS_TAG);
        }

        if( mpISP == NULL && SUPPORT_ISP_HAL )
        {
            mpISP = MAKE_HalISP(mSensorIndex, PIPE_CLASS_TAG);
        }
        P2_CAM_TRACE_END(TRACE_ADVANCED);

        if (pipeUsage.support3DNR())
        {
            P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "CamContext::init3DNR");
            mp3dnr = NSCam::NSIoPipe::NSPostProc::Hal3dnrBase::createInstance(
                MODULE_NAME, mSensorIndex);
            mp3dnr->init(MODULE_NAME);

            P2_CAM_TRACE_END(TRACE_ADVANCED);
        }

        mIsInited = MTRUE;
    }

    TRACE_FUNC_EXIT();
}


// This function shall be called only by ~P2CamContext().
// As nobody will know when the last instance will be released,
// all instances are managed by sp<>.
void P2CamContext::uninit()
{
    TRACE_FUNC_ENTER();

    if (mIsInited)
    {
        if (mp3dnr != NULL)
        {
            mp3dnr->uninit(MODULE_NAME);
            mp3dnr->destroyInstance(MODULE_NAME, mSensorIndex);
            mp3dnr = NULL;
        }

        if (mp3A != NULL)
        {
            mp3A->destroyInstance(PIPE_CLASS_TAG);
            mp3A = NULL;
        }
        if (mpISP != NULL)
        {
            mpISP->destroyInstance(PIPE_CLASS_TAG);
            mpISP = NULL;
        }

        mIsInited = MFALSE;
    }

    TRACE_FUNC_EXIT();
}


sp<P2CamContext> getP2CamContext(MUINT32 sensorIndex)
{
    return P2CamContext::getInstance(sensorIndex);
}


};
};
};

