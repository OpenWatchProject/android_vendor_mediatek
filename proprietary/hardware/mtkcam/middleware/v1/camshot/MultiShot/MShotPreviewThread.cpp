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

#define LOG_TAG "MtkCam/MShotPrvThread"

#include <mtkcam/utils/std/Log.h>
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __func__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
#include <mtkcam/def/common.h>
#include <mtkcam/middleware/v1/common.h>
using namespace android;
//
#include <mtkcam/middleware/v1/camshot/_callbacks.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/IMetadata.h>
using namespace NSCam;

#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
using namespace NSCam::v1::NSLegacyPipeline;

#include <mtkcam/utils/hw/HwInfoHelper.h>
using namespace NSCamHW;

#include "../inc/CamShotImp.h"
#include "../inc/MultiShot.h"
#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

#define MSHOTPREVIEWTHREAD_NAME       ("Cam@v1MShotPreviewThread")
#define MSHOTPREVIEWTHREAD_POLICY     (SCHED_OTHER)
#define MSHOTPREVIEWTHREAD_PRIORITY   (0)


/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {

/******************************************************************************
 *
 ******************************************************************************/
MultiShot::MShotPreviewThread::
MShotPreviewThread()
    : mpPipeline(NULL)
    , mbNeedPrecap(MFALSE)
    , mRequestNumber(0)
    , mRequestNumberMin(0)
    , mRequestNumberMax(0)
{
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MultiShot::MShotPreviewThread::
threadLoop()
{
    if ( !exitPending() ) {

        Mutex::Autolock _l(mLock);

        IMetadata settingAppMeta = mAppMeta;
        IMetadata settingHalMeta = mHalMeta;
        if( mbNeedPrecap && mRequestNumber == mRequestNumberMin+2 )
        {
            IMetadata::IEntry entry(MTK_CONTROL_AE_PRECAPTURE_TRIGGER);
            entry.push_back(MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START, Type2Type< MUINT8 >());
            settingAppMeta.update(entry.tag(), entry);
            //
            MY_LOGD("trigger AE precapture, request(%d)", mRequestNumber);
            mbNeedPrecap = MFALSE;
        }
        //
        sp< ILegacyPipeline > pPipeline = mpPipeline.promote();
        if ( pPipeline == NULL) {
            MY_LOGE("LegacyPipeline promote fail.");
            return false;
        }
        //
        if( pPipeline->submitSetting( mRequestNumber, settingAppMeta, settingHalMeta) != OK )
        {
            MY_LOGW("submit setting failed, reqNum(%d)", mRequestNumber);
        }
        mRequestNumber++;
        if( mRequestNumber > mRequestNumberMax )
        {
            mRequestNumber = mRequestNumberMin;
        }
        return true;
    }

    MY_LOGI("RequestThread exit.");
    return false;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
MultiShot::MShotPreviewThread::
start(
    wp< ILegacyPipeline >   apPipeline,
    MINT32                  aStartRequestNumber,
    MINT32                  aEndRequestNumber,
    IMetadata&              aAppMeta,
    IMetadata&              aHalMeta
)
{
    FUNC_START;

    Mutex::Autolock _l(mLock);

    mpPipeline = apPipeline;

    mRequestNumber = aStartRequestNumber;
    mRequestNumberMin = aStartRequestNumber;
    mRequestNumberMax = aEndRequestNumber;

    mAppMeta = aAppMeta;
    mHalMeta = aHalMeta;

    //normal cshot with flash need to trigger precap for faster AE converge
    {
        MUINT8 flashMode = 0, controlAeMode = 0;
        tryGetMetadata< MUINT8 >(&aAppMeta, MTK_FLASH_MODE, flashMode);
        tryGetMetadata< MUINT8 >(&aAppMeta, MTK_CONTROL_AE_MODE, controlAeMode);
        if( flashMode == MTK_FLASH_MODE_TORCH && controlAeMode == MTK_CONTROL_AE_MODE_ON )
        {
            mbNeedPrecap = MTRUE;
        }
        else
        {
            mbNeedPrecap = MFALSE;
        }
        MY_LOGD("flashMode(%d) controlAeMode(%d) need precap(%d)", flashMode, controlAeMode, mbNeedPrecap);
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MultiShot::MShotPreviewThread::
stop()
{
    FUNC_START;

    mpPipeline = NULL;
    Thread::requestExitAndWait();

    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
void
MultiShot::MShotPreviewThread::
onLastStrongRef(const void* /*id*/)
{
    FUNC_START;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
MultiShot::MShotPreviewThread::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)MSHOTPREVIEWTHREAD_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, MSHOTPREVIEWTHREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, MSHOTPREVIEWTHREAD_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "tid(%d) policy(%d) priority(%d)"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
